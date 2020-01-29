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
	Tree* find_tree(string name);
	Tree* open_tree(string path);
	void close_tree(string path);
	
	//void insert_leaf(string name);
	//void insert_leaf(tree_t tree);
	//void erase_leaf(string name);
	//void erase_leaf(tree_t tree);
	//find_leaf();
			
	void bloom(string path);
	void fold();
	
	//////////// Private? ////////////
	
	Tree* FOREST;
	bool blossomed = false;
	
	// Other
	string read_leaf_item(file_data_t item);
	
	// Root methods
	void open_root();
	void close_root();
	
	// Cache
	namespace cache{
		void init_cache();
		void release_cache();
		ListCache<string, void_shared> tree_cache;
		ListCache<string, void_shared> leaf_cache;
		ListCache<string, void_shared> intr_cache;
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::shared_future<void_shared> > tree_cache_q;
		std::unordered_map<string, std::shared_future<void_shared> > intr_cache_q, leaf_cache_q;
		std::unordered_map<string, std::pair<void_shared, int_a> > tree_cache_r;
		std::unordered_map<string, std::pair<void_shared, int_a> > intr_cache_r, leaf_cache_r;
		std::unordered_map<int_t, string> tree_cache_f;
	}
}

#endif //FOREST_H
