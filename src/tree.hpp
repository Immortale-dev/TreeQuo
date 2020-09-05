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
#include "savior.hpp"

namespace forest{
	
	class LeafRecord;
	class Savior;
	
	extern Savior* savior;
	
	class Tree{
		
		friend LeafRecord;
		friend Savior;
		friend tree_t;
		
		using tree_node_type = tree_t::Node::nodes_type;
		using tree_keys_type = tree_t::Node::keys_type;
		
		public:
			Tree();
			Tree(string path);
			Tree(string path, TREE_TYPES type, int factor);
			~Tree();
			
			string get_name();
			
			TREE_TYPES get_type();
			void set_type(TREE_TYPES type);
			
			tree_t* get_tree();
			void set_tree(tree_t* tree);
			
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
			static DBFS::File* create_leaf_file(string filename, bool lock_for_limit=false);
			
			// Tree methods
			tree_base_read_t read_base(string filename);
			static void seed_tree(DBFS::File* file, TREE_TYPES type, int factor);
			void tree_reserve();
			void tree_release();
		
			// Proceed
			void d_enter(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
			void d_leave(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
			void d_insert(tree_t::node_ptr& node);
			void d_remove(tree_t::node_ptr& node);
			void d_reserve(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
			void d_release(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
			void d_before_move(tree_t::childs_type_iterator& item, int_t step);
			void d_after_move(tree_t::childs_type_iterator& item, int_t step);
			void d_item_reserve(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type);
			void d_item_release(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type);
			void d_item_move(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item);
			void d_leaf_insert(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item);
			void d_leaf_delete(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item);
			void d_leaf_split(tree_t::node_ptr& node, tree_t::node_ptr& new_node, tree_t::node_ptr& link_node);
			void d_leaf_join(tree_t::node_ptr& node, tree_t::node_ptr& join_node, tree_t::node_ptr& link_node);
			void d_leaf_shift(tree_t::node_ptr& node, tree_t::node_ptr& shift_node);
			void d_leaf_lock(tree_t::node_ptr& node);
			void d_leaf_free(tree_t::node_ptr& node);
			void d_leaf_ref(tree_t::node_ptr& node, tree_t::node_ptr& ref_node, tree_t::LEAF_REF ref);
			void d_save_base(tree_t::node_ptr& node);
			
			// Getters
			tree_t::node_ptr get_intr(string path);
			tree_t::node_ptr get_leaf(string path);
			tree_t::node_ptr get_original(tree_t::node_ptr node);
			tree_t::node_ptr extract_node(tree_t::child_item_type_ptr item);
			tree_t::node_ptr extract_locked_node(tree_t::child_item_type_ptr item, bool w_prior=false);
			
			// Writers
			static void write_intr(DBFS::File* file, tree_intr_read_t data);
			static void write_base(DBFS::File* file, tree_base_read_t data);
			static void write_leaf(std::shared_ptr<DBFS::File> file, tree_leaf_read_t data);
			static void write_leaf_item(std::shared_ptr<DBFS::File> file, tree_t::val_type& data);
			
			// Other
			tree_t::node_ptr create_node(string path, NODE_TYPES node_type);
			tree_t::node_ptr create_node(string path, NODE_TYPES node_type, bool empty);
			
			tree_t* tree;
			TREE_TYPES type;
			string name;
			mutex tree_m;
	};
}

#endif //FOREST_TREE_H
