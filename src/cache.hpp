#ifndef FOREST_CACHE
#define FOREST_CACHE

#include <future>
#include "variables.hpp"
#include "dbutils.hpp"
#include "listcache.hpp"
#include "node_data.hpp"
#include "savior.hpp"

namespace forest{
	
	class Savior;
	
	extern Savior* savior;
	
	// Cache
	namespace cache{
		
		struct leaf_cache_ref_t{
			tree_t::node_ptr first;
			int second;
			//int items;
		};
		
		void init_cache();
		void release_cache();
		void check_leaf_ref(string& key);
		void check_intr_ref(string& key);
		void check_tree_ref(string& key);
		void set_tree_cache_length(int length);
		void set_intr_cache_length(int length);
		void set_leaf_cache_length(int length);
		void insert_item(tree_t::child_item_type_ptr& item);
		void remove_item(tree_t::child_item_type_ptr& item);
		
		void intr_lock();
		void intr_unlock();
		void leaf_lock();
		void leaf_unlock();
		std::lock_guard<mutex> get_intr_lock();
		std::lock_guard<mutex> get_leaf_lock();
		
		void reserve_node(tree_t::node_ptr& node, bool w_lock=false);
		void release_node(tree_t::node_ptr& node, bool w_lock=false);
		void reserve_intr_node(string& path);
		void release_intr_node(string& path);
		void reserve_leaf_node(string& path);
		void release_leaf_node(string& path);
		
		void intr_insert(tree_t::node_ptr& node, bool w_lock=false);
		void leaf_insert(tree_t::node_ptr& node, bool w_lock=false);
		
		void with_lock(NODE_TYPES type, std::function<void()> fn);
		
		void clear_node_cache(tree_t::node_ptr& node);
		
		
		void _intr_insert(tree_t::node_ptr& node);
		void _leaf_insert(tree_t::node_ptr& node);
		
		extern ListCache<string, tree_ptr> tree_cache;
		extern ListCache<string, tree_t::node_ptr> leaf_cache, intr_cache;
		extern mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		extern std::unordered_map<string, std::pair<tree_ptr, int> > tree_cache_r;
		extern std::unordered_map<string, leaf_cache_ref_t> leaf_cache_r;
		extern std::unordered_map<string, std::pair<tree_t::node_ptr, int> > intr_cache_r;
		
		extern std::unordered_set<string> tree_cache_q;
		extern std::condition_variable tree_cv;
		extern std::mutex tree_cv_m;
		
		extern std::queue<string> savior_save;
		extern std::mutex savior_save_m;
		//extern std::unordered_map<uintptr_t, std::unordered_map<int, int> > leaf_cache_i;
	}
}


inline void forest::cache::insert_item(tree_t::child_item_type_ptr& item)
{
	//======//log_info_private("[cache::insert_item] ("+path+":"+std::to_string(pos)+") start");
	item->item->second->res_c++;
	//======//log_info_private("[cache::insert_item] end");
}

inline void forest::cache::remove_item(tree_t::child_item_type_ptr& item)
{
	//======//log_info_private("[cache::remove_item] ("+path+":"+std::to_string(pos)+") start");
	item->item->second->res_c--;
	//======//log_info_private("[cache::remove_item] end");
}

inline void forest::cache::intr_lock()
{
	//======//log_info_private("[cache::intr_lock] start");
	intr_cache_m.lock();
	//======//log_info_private("[cache::intr_lock] end");
}

inline void forest::cache::intr_unlock()
{
	//======//log_info_private("[cache::intr_unlock] start");
	intr_cache_m.unlock();
	//======//log_info_private("[cache::intr_unlock] end");
}

inline void forest::cache::leaf_lock()
{
	//======//log_info_private("[cache::leaf_lock] start");
	leaf_cache_m.lock();
	//======//log_info_private("[cache::leaf_lock] end");
}

inline std::lock_guard<std::mutex> forest::cache::get_intr_lock()
{
	return std::lock_guard<std::mutex>(intr_cache_m);
}

inline std::lock_guard<std::mutex> forest::cache::get_leaf_lock()
{
	return std::lock_guard<std::mutex>(leaf_cache_m);
}


inline void forest::cache::reserve_intr_node(string& path)
{
	//======//log_info_private("[cache::reserve_intr_node] ("+path+") start");
	intr_cache_r[path].second++;
	//======//log_info_private("[cache::reserve_intr_node] end");
}

inline void forest::cache::release_intr_node(string& path)
{
	//======//log_info_private("[cache::release_intr_node] ("+path+") start");
	///assert(intr_cache_r.count(path));
	///assert(intr_cache_r[path].second>0);
	if(--intr_cache_r[path].second == 0){
		check_intr_ref(path);
	}
	//======//log_info_private("[cache::release_intr_node] end");
}

inline void forest::cache::reserve_leaf_node(string& path)
{
	//======//log_info_private("[cache::reserve_leaf_node] ("+path+") start");
	leaf_cache_r[path].second++;
	//======//log_info_private("[cache::reserve_leaf_node] end");
}

inline void forest::cache::release_leaf_node(string& path)
{
	//======//log_info_private("[cache::release_leaf_node] ("+path+") start");
	///assert(leaf_cache_r.count(path));
	///assert(leaf_cache_r[path].second>0);
	if(--leaf_cache_r[path].second == 0){;
		check_leaf_ref(path);
	}
	//======//log_info_private("[cache::release_leaf_node] end");
}

inline void forest::cache::_intr_insert(tree_t::node_ptr& node)
{
	string& path = get_node_data(node)->path;
	get_data(node).is_original = true;
	cache::intr_cache_r[path] = std::make_pair(node, 0);
	cache::intr_cache.push(path, node);
}

inline void forest::cache::_leaf_insert(tree_t::node_ptr& node)
{
	string& path = get_node_data(node)->path;
	get_data(node).is_original = true;
	cache::leaf_cache_r[path] = {node, 0};
	cache::leaf_cache.push(path, node);
}


#endif // FOREST_CACHE
