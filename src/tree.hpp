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

#include "dbutils.hpp"
#include "variables.hpp"
#include "cache.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "node_data.hpp"
#include "lock.hpp"

namespace forest{
	
	class Tree{
		
		using child_lengths_vec_ptr = std::vector<uint_t>*;
		using child_keys_vec_ptr = std::vector<tree_t::key_type>*;
		using child_values_vec_ptr = std::vector<tree_t::val_type>*;
		using child_nodes_vec_ptr = std::vector<string>*;
		
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
			child_nodes_vec_ptr child_values;
		};
		struct tree_base_read_t {
			TREE_TYPES type;
			NODE_TYPES branch_type;
			uint_t count;
			int factor;
			string branch;
		};
		
		
		public:
			Tree(string path);
			~Tree();
			
			string get_name();
			
			tree_t* get_tree();
			void insert(tree_t::key_type key, tree_t::val_type val, bool update=false);
			void erase(tree_t::key_type key);
			tree_t::iterator find(tree_t::key_type key);
			
			static string seed(TREE_TYPES type, int factor);
			static string seed(TREE_TYPES type, string path, int factor);
			static tree_ptr get(string path);
			
		private:
		
			// Intr methods
			tree_intr_read_t read_intr(string filename);
			void materialize_intr(tree_t::node_ptr node);
			void unmaterialize_intr(tree_t::node_ptr node);
			
			// Leaf methods
			tree_leaf_read_t read_leaf(string filename);
			void materialize_leaf(tree_t::node_ptr node);
			void unmaterialize_leaf(tree_t::node_ptr node);
			
			// Tree base methods
			tree_base_read_t read_base(string filename);
			static void seed_tree(DBFS::File* file, TREE_TYPES type, int factor);
		
			// Proceed
			void d_enter(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_leave(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_insert(tree_t::node_ptr node, tree_t* tree);
			void d_remove(tree_t::node_ptr node, tree_t* tree);
			void d_reserve(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_release(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_before_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree);
			void d_after_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree);
			void d_item_reserve(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_item_release(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree);
			void d_item_move(tree_t::node_ptr node, bool release, tree_t* tree);
			void d_leaf_insert(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree);
			void d_leaf_delete(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree);
			void d_leaf_split(tree_t::node_ptr node, tree_t::node_ptr new_node, tree_t::node_ptr link_node, tree_t* tree);
			void d_leaf_join(tree_t::node_ptr node, tree_t::node_ptr join_node, tree_t::node_ptr link_node, tree_t* tree);
			void d_leaf_shift(tree_t::node_ptr node, tree_t::node_ptr shift_node, tree_t* tree);
			void d_leaf_free(tree_t::node_ptr node, tree_t* tree);
			void d_leaf_ref(tree_t::node_ptr node, tree_t::node_ptr ref_node, tree_t::LEAF_REF ref, tree_t* tree);
			void d_save_base(tree_t::node_ptr node, tree_t* tree);
			
			// Getters
			tree_t::node_ptr get_intr(string path);
			tree_t::node_ptr get_leaf(string path);
			tree_t::node_ptr get_original(tree_t::node_ptr node);
			//tree_ptr get_tree(string path);
			
			// Writers
			void write_intr(DBFS::File* file, tree_intr_read_t data);
			void write_base(DBFS::File* file, tree_base_read_t data);
			void write_leaf(std::shared_ptr<DBFS::File> file, tree_leaf_read_t data);
			void write_leaf_item(std::shared_ptr<DBFS::File> file, tree_t::val_type& data);
			
			// Other
			//tree_t::node_ptr unvoid_node(void_shared node);
			void clear_node_cache(tree_t::node_ptr node);
			driver_t* init_driver();
			tree_t::node_ptr create_node(string path, NODE_TYPES node_type);
			TREE_TYPES get_type();
			
			tree_t* tree;
			TREE_TYPES type;
			mutex tree_m;
			string name;
	};
}

#endif //FOREST_TREE_H
