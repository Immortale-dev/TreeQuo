#ifndef DB_H
#define DB_H

#include <iostream>

#include <string>
#include <exception>
#include <unordered_map>
#include <utility>
#include <memory>
#include <mutex>
#include <future>
#include "bplustree.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"
#include "dbdriver.hpp"

class DB{
	
	enum class KEY_TYPES { INT, STRING };
	enum class NODE_TYPES { INTR, LEAF };
	
	using string = std::string;
	using int_t = long long int;
	using file_pos_t = long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	
	using driver_root_t = DBDriver<string, string>;
	using driver_int_t = DBDriver<int_t, file_pos_t>;
	using driver_string_t = DBDriver<string, file_pos_t>;
	
	using root_tree_t = BPlusTree<string, string, driver_root_t>;
	using int_tree_t = BPlusTree<int_t, file_pos_t, driver_int_t>;
	using string_tree_t = BPlusTree<string, file_pos_t, driver_string_t>;
	
	struct qtree_t {
		void_shared tree;
		TREE_TYPES type;
	};
	struct qnode_t {
		void_shared node;
		TREE_TYPES type;
	};
	struct qintr_t {
		void* child_keys;
		void* child_values;
		TREE_TYPES type;
	};
	struct qtree_base_t {
		int_t count;
		int factor;
		TREE_TYPES type;
		string branch;
		bool is_leaf;
	};
	struct qtree_intr_node_t {
		
	};
	struct qtree_leaf_node_t {
		
	};
	struct qnode_data_t {
		bool ghost = true;
		string path = "";
		qnode_data_t(bool ghost, string path) : ghost(ghost), path(path) {};
	};
	
	using qnode_data_ptr = std::shared_ptr<qnode_data_t>;

	public:
		DB();
		~DB();
		DB(string path);
		
		void create_qtree(TREE_TYPES type, string name);
		void delete_qtree(string name);
		qtree_t open_qtree(string name);
		
		void insert_qleaf(string name);
		void insert_qleaf(qtree_t tree);
		void erase_qleaf(string name);
		void erase_qleaf(qtree_t tree);
		
		//find_qleaf();
				
		void bloom(string path);
		void fold(bool cut);
		
	private:
		root_tree_t* FOREST;
		bool blossomed = false;
		
		root_tree_t* open_root();
		qtree_base_t read_base(string filename);
		
		void create_root_file();
		string create_qtree_base(TREE_TYPES type);
		void insert_qtree(string name, string file_name, TREE_TYPES type);
		void erase_qtree(string name);
		
		// Create Tree Node smart ptr
		template<typename T>
		typename T::node_ptr create_node(string path, bool is_leaf);
		
		qnode_data_ptr create_node_data(bool ghost, string path);
		
		// Cache
		std::unordered_map<string, qtree_t> opened_trees;
		ListCache<string, qtree_t> tree_cache;
		ListCache<string, qnode_t> leaf_cache;
		ListCache<string, qintr_t> node_cache;
		mutex tree_cache_m, leaf_cache_m, node_cache_m;
		std::unordered_map<string, std::shared_future<qtree_t> > tree_cache_q;
		
		// Drivers
		void init_drivers();
		driver_root_t* driver_root;
		driver_int_t* driver_int;
		driver_string_t* driver_string;
		
		// Proceed
		template<typename T>
		void d_enter(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_leave(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_insert(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_remove(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_reserve(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_release(typename T::node_ptr node, T* tree);
		template<typename T>
		void d_before_move(typename T::iterator& it, typename T::node_ptr node, int_t step, T* tree);
		template<typename T>
		void d_after_move(typename T::iterator& it, typename T::node_ptr node, int_t step, T* tree);
	
		// Properties
		const int DEFAULT_FACTOR = 100;
		const string ROOT_TREE = "_root";
};

template<typename T>
typename T::node_ptr DB::create_node(string path, bool is_leaf)
{
	typename T::Node* node;
	if(!is_leaf){
		node = new typename T::InternalNode();
	} else {
		node = new typename T::LeafNode();
	}
	node->data = create_node_data(true, path);
	return typename T::node_ptr(node);
}

template<typename T>
void DB::d_enter(typename T::node_ptr node, T* tree)
{
	std::cout << "ENTER" << std::endl;
}

template<typename T>
void DB::d_leave(typename T::node_ptr node, T* tree)
{
	std::cout << "LEAVE" << std::endl;
}

template<typename T>
void DB::d_insert(typename T::node_ptr node, T* tree)
{
	std::cout << "INSERT" << std::endl;
}

template<typename T>
void DB::d_remove(typename T::node_ptr node, T* tree)
{
	std::cout << "REMOVE" << std::endl;
}

template<typename T>
void DB::d_reserve(typename T::node_ptr node, T* tree)
{
	std::cout << "RESERVE" << std::endl;
}

template<typename T>
void DB::d_release(typename T::node_ptr node, T* tree)
{
	std::cout << "RELEASE" << std::endl;
}

template<typename T>
void DB::d_before_move(typename T::iterator& it, typename T::node_ptr node, int_t step, T* tree)
{
	std::cout << "BEFORE_MOVE" << std::endl;
}

template<typename T>
void DB::d_after_move(typename T::iterator& it, typename T::node_ptr node, int_t step, T* tree)
{
	std::cout << "AFTER_MOVE" << std::endl;
}

#endif // DB_H
