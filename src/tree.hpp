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
		
		enum class KEY_TYPES { STRING };
		enum class NODE_TYPES { INTR, LEAF };
		
		struct leaf_t {
			TREE_TYPES type;
			void* child_keys;
			void* child_lengths;
			int_t start_data;
			DBFS::File* file;
			string left_leaf, right_leaf;
		};
		struct intr_t {
			TREE_TYPES type;
			NODE_TYPES childs_type;
			void* child_keys;
			void* child_values;
		};
		struct tree_base_read_t {
			int_t count;
			int factor;
			TREE_TYPES type;
			string branch;
			NODE_TYPES branch_type;
		};
		struct node_data_t {
			bool ghost = true;
			string path = "";
			string prev = "-";
			string next = "-";
			node_data_t(bool ghost, string path) : ghost(ghost), path(path) {};
		};
		
		using node_data_ptr = std::shared_ptr<node_data_t>;
		
	}
	
}

#endif //FOREST_TREE_H
