#ifndef FOREST_CACHE
#define FOREST_CACHE

#include <future>
#include "variables.hpp"
#include "dbutils.hpp"
#include "listcache.hpp"
#include "node_data.hpp"

namespace forest{
	// Cache
	namespace cache{
		void init_cache();
		void release_cache();
		void check_leaf_ref(string key);
		void check_intr_ref(string key);
		void check_tree_ref(string key);
		void set_tree_cache_length(int length);
		void set_intr_cache_length(int length);
		void set_leaf_cache_length(int length);
		void insert_item(string path, int pos);
		void remove_item(string path, int pos);
		
		void intr_lock();
		void intr_unlock();
		void leaf_lock();
		void leaf_unlock();
		std::lock_guard<mutex> get_intr_lock();
		std::lock_guard<mutex> get_leaf_lock();
		
		void reserve_node(tree_t::node_ptr node, bool w_lock=false);
		void release_node(tree_t::node_ptr node, bool w_lock=false);
		void reserve_intr_node(string& path);
		void release_intr_node(string& path);
		void reserve_leaf_node(string& path);
		void release_leaf_node(string& path);
		
		void intr_insert(tree_t::node_ptr node, bool w_lock=false);
		void leaf_insert(tree_t::node_ptr node, bool w_lock=false);
		
		void with_lock(NODE_TYPES type, std::function<void()> fn);
		
		
		void _intr_insert(tree_t::node_ptr node);
		void _leaf_insert(tree_t::node_ptr node);
		
		
		extern ListCache<string, tree_ptr> tree_cache;
		extern ListCache<string, tree_t::node_ptr> leaf_cache, intr_cache;
		extern mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		extern std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		extern std::unordered_map<string, std::shared_future<tree_t::node_ptr> > intr_cache_q, leaf_cache_q;
		extern std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		extern std::unordered_map<string, std::pair<tree_t::node_ptr, int_a> > intr_cache_r, leaf_cache_r;
		extern std::unordered_map<int_t, string> tree_cache_f;
		extern std::unordered_map<string, std::unordered_map<int, int> > leaf_cache_i;
	}
}

#endif // FOREST_CACHE
