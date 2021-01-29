#ifndef FOREST_CACHE_H
#define FOREST_CACHE_H

#include <future>
#include "variables.hpp"
#include "dbutils.hpp"
#include "listcache.hpp"
#include "node_data.hpp"
#include "savior.hpp"
#include "tree.hpp"

namespace forest{
namespace details{

	class Savior;
	
	extern Savior* savior;
	
	// Cache
	namespace cache{

		struct node_cache_ref_t{
			tree_t::node_ptr first;
			int second;
		};
		
		struct tree_cache_ref_t{
			tree_ptr first;
			int second;
		};
		
		using list_cache_iterator = std::list<node_ptr>::iterator;
		
		void init_cache();
		void release_cache();
		void check_leaf_ref(node_ptr node);
		void check_intr_ref(node_ptr node);
		void check_tree_ref(tree_ptr tree);
		void set_tree_cache_length(int length);
		void set_intr_cache_length(int length);
		void set_leaf_cache_length(int length);
		void insert_item(tree_t::child_item_type_ptr& item);
		void remove_item(tree_t::child_item_type_ptr& item);
		
		void leaf_cache_push(node_ptr node);
		void intr_cache_push(node_ptr node);
		void tree_cache_push(tree_ptr tree);
		void leaf_cache_remove(node_ptr node);
		void intr_cache_remove(node_ptr node);
		void tree_cache_remove(tree_ptr tree);
		void leaf_cache_clear();
		void intr_cache_clear();
		void tree_cache_clear();
		
		void intr_lock();
		void intr_unlock();
		void leaf_lock();
		void leaf_unlock();
		void tree_lock();
		void tree_unlock();
		std::lock_guard<mutex> get_intr_lock();
		std::lock_guard<mutex> get_leaf_lock();
		
		void reserve_node(tree_t::node_ptr& node, bool w_lock=false);
		void release_node(tree_t::node_ptr& node, bool w_lock=false);
		void reserve_intr_node(node_ptr node, int cnt = 1);
		void release_intr_node(node_ptr node, int cnt = 1);
		void reserve_leaf_node(node_ptr node, int cnt = 1);
		void release_leaf_node(node_ptr node, int cnt = 1);
		void reserve_tree(tree_ptr tree);
		void release_tree(tree_ptr tree);
		
		void intr_insert(tree_t::node_ptr& node, bool w_lock=false);
		void leaf_insert(tree_t::node_ptr& node, bool w_lock=false);
		
		void with_lock(NODE_TYPES type, std::function<void()> fn);
		
		void clear_node_cache(tree_t::node_ptr& node);
		
		void _intr_insert(tree_t::node_ptr& node);
		void _leaf_insert(tree_t::node_ptr& node);
		
		//extern ListCache<string, tree_ptr> tree_cache;
		//extern ListCache<string, tree_t::node_ptr> leaf_cache, intr_cache;
		extern mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		extern std::unordered_map<string, tree_cache_ref_t*> tree_cache_r;
		extern std::unordered_map<string, node_cache_ref_t*> leaf_cache_r;
		extern std::unordered_map<string, node_cache_ref_t*> intr_cache_r;
		
		extern std::unordered_set<string> tree_cache_q;
		extern std::condition_variable tree_cv;
		extern std::mutex tree_cv_m;
		
		extern std::queue<string> savior_save;
		extern std::mutex savior_save_m;
	}
	
} // details
} // forest


inline void forest::details::cache::insert_item(tree_t::child_item_type_ptr& item)
{
	++item->item->second->res_c;
}

inline void forest::details::cache::remove_item(tree_t::child_item_type_ptr& item)
{
	ASSERT(item->item->second->res_c > 0);
	--item->item->second->res_c;
}

inline void forest::details::cache::intr_lock()
{
	intr_cache_m.lock();
}

inline void forest::details::cache::intr_unlock()
{
	intr_cache_m.unlock();
}

inline void forest::details::cache::leaf_lock()
{
	leaf_cache_m.lock();
}

inline void forest::details::cache::tree_lock()
{
	tree_cache_m.lock();
}

inline void forest::details::cache::tree_unlock()
{
	tree_cache_m.unlock();
}

inline std::lock_guard<std::mutex> forest::details::cache::get_intr_lock()
{
	return std::lock_guard<std::mutex>(intr_cache_m);
}

inline std::lock_guard<std::mutex> forest::details::cache::get_leaf_lock()
{
	return std::lock_guard<std::mutex>(leaf_cache_m);
}

inline void forest::details::cache::reserve_intr_node(node_ptr node, int cnt)
{
	assert(get_data(node).is_original);
	get_data(node).cached_ref->second += cnt;
}

inline void forest::details::cache::release_intr_node(node_ptr node, int cnt)
{
	assert(get_data(node).is_original);
	auto& data = get_data(node);
	ASSERT(data.cached_ref->second >= cnt);
	data.cached_ref->second -= cnt;
	if(data.cached_ref->second == 0){
		check_intr_ref(node);
	}
}

inline void forest::details::cache::reserve_leaf_node(node_ptr node, int cnt)
{
	assert(get_data(node).is_original);
	get_data(node).cached_ref->second += cnt;
}

inline void forest::details::cache::release_leaf_node(node_ptr node, int cnt)
{
	assert(get_data(node).is_original);
	auto& data = get_data(node);
	ASSERT(data.cached_ref->second >= cnt);
	data.cached_ref->second -= cnt;
	if(data.cached_ref->second == 0){
		check_leaf_ref(node);
	}
}

inline void forest::details::cache::_intr_insert(tree_t::node_ptr& node)
{
	string& path = get_node_data(node)->path;
	auto* cache_obj = new node_cache_ref_t{node, 0};
	
	auto& node_data = get_data(node);
	node_data.is_original = true;
	node_data.cached_ref = cache_obj;
	
	cache::intr_cache_r[path] = cache_obj;
	cache::intr_cache_push(node);
}

inline void forest::details::cache::_leaf_insert(tree_t::node_ptr& node)
{
	string& path = get_node_data(node)->path;
	auto* cache_obj = new node_cache_ref_t{node, 0};
	
	auto& node_data = get_data(node);
	node_data.is_original = true;
	node_data.cached_ref = cache_obj;
	
	cache::leaf_cache_r[path] = cache_obj;
	cache::leaf_cache_push(node);
}

#endif // FOREST_CACHE_H
