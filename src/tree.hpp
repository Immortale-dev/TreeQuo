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

#ifdef DEBUG_PERF
extern unsigned long int h_enter, h_leave, h_insert, h_remove, h_reserve,
		h_release, h_l_insert, h_l_delete, h_l_split, h_l_join,
		h_l_shift, h_l_lock, h_l_free, h_l_ref, h_save_base; 
#endif

namespace forest{
namespace details{
	
	class LeafRecord;
	class Savior;
	
	extern Savior* savior;
	
	class Tree{
		
		friend LeafRecord;
		friend Savior;
		friend tree_t;
		
		using tree_node_type = tree_t::Node::nodes_type;
		using tree_keys_type = tree_t::Node::keys_type;
		
		struct tree_cache_t;
		
		public:
			Tree();
			Tree(string path);
			Tree(string path, TREE_TYPES type, int factor, string annotation);
			~Tree();
			
			string get_name();
			void set_name(string name);
			
			void lock();
			void unlock();
			void ready();
			
			string get_annotation();
			void set_annotation(string annotation);
			
			tree_cache_t& get_cached();
			
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
			
			void tree_reserve();
			void tree_release();
			
		private:
		
			// Cache optimisation structure
			struct tree_cache_t{
				cache::tree_cache_ref_t* ref = nullptr;
				bool iterator_valid = false;
				std::list<tree_ptr>::iterator iterator;
			};
		
			// Intr methods
			tree_intr_read_t read_intr(string filename);
			void materialize_intr(tree_t::node_ptr node);
			void unmaterialize_intr(tree_t::node_ptr node);
			
			// Leaf methods
			tree_leaf_read_t read_leaf(string filename);
			void materialize_leaf(tree_t::node_ptr node);
			void unmaterialize_leaf(tree_t::node_ptr node);
			
			// Tree methods
			static tree_base_read_t read_base(string filename);
			static void seed_tree(DBFS::File* file, TREE_TYPES type, int factor);
		
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
			void d_offset_reserve(tree_t::node_ptr& node, int offset);
			void d_offset_release(tree_t::node_ptr& node, int offset);
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
			
			// Savers
			static void save_intr(node_ptr node, DBFS::File* f);
			static void save_leaf(node_ptr node, file_ptr fp);
			static void save_base(tree_ptr tree, DBFS::File* f);
			
			// Writers
			static void write_intr(DBFS::File* file, tree_intr_read_t data);
			static void write_base(DBFS::File* file, tree_base_read_t data);
			static void write_leaf(file_ptr file, tree_leaf_read_t data);
			static void write_leaf_item(file_ptr file, tree_t::val_type& data);
			
			// Other
			static tree_t::node_ptr create_node(string path, NODE_TYPES node_type);
			static tree_t::node_ptr create_node(string path, NODE_TYPES node_type, bool empty);
			
			tree_t* tree;
			TREE_TYPES type;
			string name;
			string annotation;
			mutex tree_m;
			
			tree_cache_t cached;
	};
	
} // details
} // forest

#endif //FOREST_TREE_H
