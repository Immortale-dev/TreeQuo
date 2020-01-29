#ifndef FOREST_TREE_H
#define FOREST_TREE_H

#include <iostream>

#include <string>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <memory>
#include <mutex>
#include <future>
#include <vector>
#include <type_traits>
#include <atomic>
#include <sstream>
#include <cassert>
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"

namespace forest{
	
	class Tree{
		
		using vector = std::vector;
		using child_lengths_vec_ptr = vector<uint_t>*;
		using child_keys_vec_ptr = vector<tree_t::key_type>*;
		using child_values_vec_ptr = vector<tree_t::val_type>*;
		
		enum class KEY_TYPES { STRING };
		enum class NODE_TYPES { INTR, LEAF };
		
		struct tree_leaf_read_t {
			child_keys_vec_ptr child_keys;
			child_lengths_vec_ptr child_lengths;
			uint_t start_data;
			DBFS::File* file;
			string left_leaf, right_leaf;
		};
		struct tree_intr_read_t {
			NODE_TYPES childs_type;
			child_keys_vec_ptr child_keys;
			child_values_vec_ptr child_values;
		};
		struct tree_base_read_t {
			TREE_TYPES type;
			NODE_TYPES branch_type;
			uint_t count;
			int factor;
			string branch;
		};
		struct node_data_t {
			bool ghost = true;
			string path = "";
			string prev = "-";
			string next = "-";
			node_data_t(bool ghost, string path) : ghost(ghost), path(path) {};
		};
		using node_data_ptr = std::shared_ptr<node_data_t>;
		
		
		public:
			Tree();
			~Tree();
			
		private:
		
			// Intr methods
			tree_intr_read_t read_intr(string filename);
			void materialize_intr(tree_t::node_ptr& node, string path);
			void unmaterialize_intr(tree_t::node_ptr& node, string path);
			void check_intr_ref(string key);
			
			// Leaf methods
			tree_leaf_read_t read_leaf(string filename);
			void materialize_leaf(tree_t::node_ptr& node, string path);
			void unmaterialize_leaf(tree_t::node_ptr& node, string path);
			void check_leaf_ref(string key);
			
			// Tree base methods
			tree_base_read_t read_base(string filename);
			void create_root_file(); // TODO: replace this method with something like "seed" (to be static method)
			string create_tree_base(TREE_TYPES type); // TODO: the same as with top one
			void insert_tree(string name, string file_name, TREE_TYPES type); // TODO: should be forest method
			void erase_tree(string path); // TODO: should be forest method
			void check_tree_ref(string key);
			
			// Node data
			node_data_ptr create_node_data(bool ghost, string path);
			node_data_ptr create_node_data(bool ghost, string path, string prev, string next);
			node_data_ptr get_node_data(void_shared d);
		
			// Proceed
			void d_enter(tree_t::node_ptr& node, tree_t* tree);
			void d_leave(tree_t::node_ptr& node, tree_t* tree);
			void d_insert(tree_t::node_ptr& node, tree_t* tree);
			void d_remove(tree_t::node_ptr& node, tree_t* tree);
			void d_reserve(tree_t::node_ptr& node, tree_t* tree);
			void d_release(tree_t::node_ptr& node, tree_t* tree);
			void d_before_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree);
			void d_after_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree);
			
			// Getters
			tree_t::node_ptr get_intr(string path);
			tree_t::node_ptr get_leaf(string path);
			tree_t* get_tree(string path);
			
			// Writers
			void write_intr(DBFS::File* file, tree_intr_read_t data);
			void write_base(DBFS::File* file, tree_base_read_t data);
			void write_leaf(std::shared_ptr<DBFS::File> file, tree_leaf_read_t data);
			void write_leaf_item(std::shared_ptr<DBFS::File> file, tree_t::val_type& data);
			
			// Other
			tree_t::node_ptr unvoid_node(void_shared node);
			void clear_node_cache(tree_t::node_ptr node);
			
			const string LEAF_NULL = "-";
			TREE_TYPES type;
		
	}
	
}

#endif //FOREST_TREE_H
