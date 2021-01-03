#ifndef FOREST_TYPES_H
#define FOREST_TYPES_H

namespace forest{
	
	enum class TREE_TYPES { KEY_STRING };
	enum class LEAF_POSITION{ BEGIN, END, LOWER, UPPER };
	enum class KEY_TYPES { STRING };
	enum class NODE_TYPES { INTR, LEAF };
	enum class SAVE_TYPES{ LEAF, INTR, BASE };
	
namespace details{
	
	extern int CACHE_BYTES;
	extern int OPENED_FILES_LIMIT;
	
	class Tree;
	class file_data_t;
	class detached_leaf;
	class tree_owner;
	
	using string = std::string;
	using int_t = long long int;
	using uint_t = unsigned long long int;
	using file_pos_t = unsigned long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	using int_a = std::atomic<int>;
	using uintptr_t = std::uintptr_t;
	
	using file_data_ptr = std::shared_ptr<file_data_t>;
	using detached_leaf_ptr = std::shared_ptr<detached_leaf>;
	using tree_owner_ptr = std::shared_ptr<tree_owner>;
	
	using tree_t = BPlusTree<string, file_data_ptr, Tree>;
	using child_item_type_ptr = tree_t::child_item_type_ptr;
	
	using tree_ptr = std::shared_ptr<Tree>;
	using node_ptr = tree_t::node_ptr;
	using file_ptr = std::shared_ptr<DBFS::File>;
	
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
		string annotation;
	};
	
} // details
} // forest

#endif // FOREST_TYPES_H
