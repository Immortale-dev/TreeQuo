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
	
	void create_tree(TREE_TYPES type, string name, int factor = 0);
	void delete_tree(string name);
	tree_ptr find_tree(string name);
	tree_ptr open_tree(string path);
	void leave_tree(string path);
	void leave_tree(tree_ptr tree);
	
	void insert_leaf(string name, tree_t::key_type key, tree_t::val_type val);
	void update_leaf(string name, tree_t::key_type key, tree_t::val_type val);
	void erase_leaf(string name, tree_t::key_type key);
	LeafRecord_ptr find_leaf(string name, tree_t::key_type key);
	LeafRecord_ptr find_leaf(string name, RECORD_POSITION position = RECORD_POSITION::BEGIN);
	LeafRecord_ptr find_leaf(string name, tree_t::key_type key, RECORD_POSITION position);
			
	void bloom(string path);
	void fold();
	
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
	
	//////////// Private? ////////////
	
	// Other
	string read_leaf_item(file_data_ptr item);
	
	// Root methods
	void open_root();
	void close_root();
	void create_root_file();
	
	// Tree methods
	void insert_tree(string name, string path);
	void insert_tree(string name, string file_name, tree_ptr tree);
	void erase_tree(string path);
	tree_ptr get_tree(string path);
}

#endif //FOREST_H
