#ifndef FOREST_H
#define FOREST_H


#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"
#include "tree.hpp"

namespace forest{
	
	using tree_ptr = std::shared_ptr<Tree>;
	
	void create_tree(TREE_TYPES type, string name);
	void delete_tree(string name);
	tree_ptr find_tree(string name);
	tree_ptr open_tree(string path);
	void close_tree(string path);
	
	void insert_leaf(string name, tree_t::key_type key, tree_t::val_type val);
	void erase_leaf(string name, tree_t::key_type key);
	file_data_t find_leaf(string name, tree_t::key_type key);
			
	void bloom(string path);
	void fold();
	
	//////////// Private? ////////////
	
	// Other
	string read_leaf_item(file_data_t item);
	
	// Root methods
	void open_root();
	void close_root();
	void create_root_file();
	
	// Tree methods
	void insert_tree(string name, string path);
	void erase_tree(string path);
	tree_ptr get_tree(string path);
	
	// Cache
	namespace cache{
		void init_cache();
		void release_cache();
		void check_leaf_ref(string key);
		void check_intr_ref(string key);
		void check_tree_ref(string key);
		
		extern ListCache<string, tree_ptr> tree_cache;
		extern ListCache<string, tree_t::node_ptr> leaf_cache, intr_cache;
		extern mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		extern std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		extern std::unordered_map<string, std::shared_future<tree_t::node_ptr> > intr_cache_q, leaf_cache_q;
		extern std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		extern std::unordered_map<string, std::pair<tree_t::node_ptr, int_a> > intr_cache_r, leaf_cache_r;
		extern std::unordered_map<int_t, string> tree_cache_f;
	}
	
	// Constants
	extern tree_ptr FOREST;
	extern bool blossomed;
	extern const int DEFAULT_FACTOR;
	extern const string ROOT_TREE;
}

#endif //FOREST_H
