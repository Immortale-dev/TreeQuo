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
#include "detached_leaf.hpp"
#include "tree_owner.hpp"

namespace forest{
	
	// Aliases
	using Leaf = details::LeafRecord_ptr;
	using Tree = details::tree_owner_ptr;
	using DetachedLeaf = details::detached_leaf_ptr;
	using LeafFile = details::file_ptr;
	using LeafKey = details::tree_t::key_type;
	using size_t = details::uint_t;
	using string = details::string;
	
	// Tree modifications
	void plant_tree(TREE_TYPES type, details::string name, int factor = 0, details::string annotation = "");
	void cut_tree(details::string name);
	Tree find_tree(details::string name);
	Tree reach_tree(details::string path);
	void leave_tree(details::string path);
	void leave_tree(Tree tree);
	
	// Tree operations with name
	void insert_leaf(details::string name, details::tree_t::key_type key, details::detached_leaf_ptr val);
	void update_leaf(details::string name, details::tree_t::key_type key, details::detached_leaf_ptr val);
	void remove_leaf(details::string name, details::tree_t::key_type key);
	Leaf find_leaf(details::string name, details::tree_t::key_type key);
	Leaf find_leaf(details::string name, LEAF_POSITION position = LEAF_POSITION::BEGIN);
	Leaf find_leaf(details::string name, details::tree_t::key_type key, LEAF_POSITION position);
	
	// Tree operations with tree
	void insert_leaf(Tree tree, details::tree_t::key_type key, details::detached_leaf_ptr val);
	void update_leaf(Tree tree, details::tree_t::key_type key, details::detached_leaf_ptr val);
	void remove_leaf(Tree tree, details::tree_t::key_type key);
	Leaf find_leaf(Tree tree, details::tree_t::key_type key);
	Leaf find_leaf(Tree tree, LEAF_POSITION position = LEAF_POSITION::BEGIN);
	Leaf find_leaf(Tree tree, details::tree_t::key_type key, LEAF_POSITION position);
	
	// Leaf Methods
	DetachedLeaf make_leaf(details::string data);
	DetachedLeaf make_leaf(char* buffer, details::uint_t length);
	DetachedLeaf make_leaf(LeafFile file, details::uint_t start, details::uint_t length);
	
			
	// Init methods
	void bloom(details::string path);
	void fold();
	
	// Status methods
	bool blooms();
	int get_save_queue_size();
	int get_opened_files_count();
	
	/* Configurations */
	void config_root_factor(int root_factor);
	void config_default_factor(int default_factor);
	void config_root_tree(details::string root_tree);
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
