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
#include "dbutils.hpp"

namespace forest{
	
	class Tree;
	using tree_ptr = std::shared_ptr<Tree>;
	using node_ptr = tree_t::node_ptr;
	
	class Tree{
		
		using child_lengths_vec_ptr = std::vector<uint_t>*;
		using child_keys_vec_ptr = std::vector<tree_t::key_type>*;
		using child_values_vec_ptr = std::vector<tree_t::val_type>*;
		using child_nodes_vec_ptr = std::vector<string>*;
		
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
			child_nodes_vec_ptr child_values;
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
			Tree(string path);
			~Tree();
			
			string get_name();
			
			tree_t* get_tree();
			void insert(tree_t::key_type key, tree_t::val_type val);
			void erase(tree_t::key_type key);
			file_data_t find(tree_t::key_type key);
			
			static string seed(TREE_TYPES type);
			static string seed(TREE_TYPES type, string path);
			static tree_ptr get(string path);
			
		private:
		
			// Intr methods
			tree_intr_read_t read_intr(string filename);
			void materialize_intr(tree_t::node_ptr& node, string path);
			void unmaterialize_intr(tree_t::node_ptr& node, string path);
			
			// Leaf methods
			tree_leaf_read_t read_leaf(string filename);
			void materialize_leaf(tree_t::node_ptr& node, string path);
			void unmaterialize_leaf(tree_t::node_ptr& node, string path);
			
			// Tree base methods
			tree_base_read_t read_base(string filename);
			static void seed_tree(DBFS::File* file, TREE_TYPES type, int factor);
			
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
			void d_save_base(tree_t::node_ptr& node, tree_t* tree);
			
			// Getters
			node_ptr get_intr(string path);
			node_ptr get_leaf(string path);
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
			node_ptr create_node(string path, NODE_TYPES node_type);
			TREE_TYPES get_type();
			
			tree_t* tree;
			TREE_TYPES type;
			mutex tree_m;
			string name;
	};
	
	
	namespace cache{
		void init_cache();
		void release_cache();
		void check_leaf_ref(string key);
		void check_intr_ref(string key);
		void check_tree_ref(string key);
		
		extern ListCache<string, tree_ptr> tree_cache;
		extern ListCache<string, node_ptr> leaf_cache, intr_cache;
		extern mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		extern std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		extern std::unordered_map<string, std::shared_future<node_ptr> > intr_cache_q, leaf_cache_q;
		extern std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		extern std::unordered_map<string, std::pair<node_ptr, int_a> > intr_cache_r, leaf_cache_r;
	}
	
	extern const string LEAF_NULL;
	extern const int DEFAULT_FACTOR;
}

#endif //FOREST_TREE_H
