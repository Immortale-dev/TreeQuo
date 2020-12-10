#ifndef FOREST_H
#define FOREST_H


#include "variables.hpp"
#include "cache.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"
#include "tree.hpp"
#include "leaf_record.hpp"
#include "savior.hpp"

namespace forest{
	
	using tree_ptr = std::shared_ptr<Tree>;
	using child_item_type_ptr = tree_t::child_item_type_ptr;
	using leaf = LeafRecord_ptr;
	using tree = tree_ptr;
	
	void plant_tree(TREE_TYPES type, string name, int factor = 0, string annotation = "");
	void cut_tree(string name);
	tree find_tree(string name);
	tree reach_tree(string path);
	void leave_tree(string path);
	void leave_tree(tree tree);
	
	// Tree operations with name
	void insert_leaf(string name, tree_t::key_type key, tree_t::val_type val);
	void update_leaf(string name, tree_t::key_type key, tree_t::val_type val);
	void remove_leaf(string name, tree_t::key_type key);
	leaf find_leaf(string name, tree_t::key_type key);
	leaf find_leaf(string name, LEAF_POSITION position = LEAF_POSITION::BEGIN);
	leaf find_leaf(string name, tree_t::key_type key, LEAF_POSITION position);
	
	// Tree operations with tree
	void insert_leaf(tree tree, tree_t::key_type key, tree_t::val_type val);
	void update_leaf(tree tree, tree_t::key_type key, tree_t::val_type val);
	void remove_leaf(tree tree, tree_t::key_type key);
	leaf find_leaf(tree tree, tree_t::key_type key);
	leaf find_leaf(tree tree, LEAF_POSITION position = LEAF_POSITION::BEGIN);
	leaf find_leaf(tree tree, tree_t::key_type key, LEAF_POSITION position);
			
	// Init methods
	void bloom(string path);
	void fold();
	
	// Status methods
	bool blooms();
	int get_save_queue_size();
	int get_opened_files_count();
	
	/* Configurations */
	void config_root_factor(int root_factor);
	void config_default_factor(int default_factor);
	void config_root_tree(string root_tree);
	void config_intr_cache_length(int length);
	void config_leaf_cache_length(int length);
	void config_tree_cache_length(int length);
	void config_cache_bytes(int bytes);
	void config_chunk_bytes(int bytes);
	void config_opened_files_limit(int count);
	void config_save_schedule_mks(int mks);
	void config_savior_queue_size(int length);
	
	//////////// Private? ////////////
	
	namespace details{
		
		// Root methods
		void open_root();
		void close_root();
		void create_root_file();
		
		// Tree methods
		void insert_tree(string name, string file_name, tree_ptr tree);
		void erase_tree(string path);
		tree_ptr get_tree(string path);
		
		// Other methods
		void init_savior();
		void release_savior();
	}
}

#endif //FOREST_H
