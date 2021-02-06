#ifndef FOREST_DBUTILS_H
#define FOREST_DBUTILS_H

// define it for saving logs and turning on asserts
//#define DEBUG
#define DEBUG_PERF

#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstdint>
//#include <iostream> //for debugging purposes

#include "log.hpp"
#include "dbfs.hpp"
#include "bplustree.hpp"
#include "dbexception.hpp"
#include "threading.hpp"
#include "types.hpp"
#include "file_data.hpp"
#include "node_addition.hpp"

namespace forest{
namespace details{
	
	// Nodes data methods
	node_addition& get_data(node_ptr node);
	node_addition& get_data(tree_t::Node* node);
	
	// Leaf methods
	file_data_ptr leaf_value(string str);
	file_data_ptr extract_leaf_val(detached_leaf_ptr dl);
	string to_string(int num);
	string read_leaf_item(file_data_ptr item);
	
	// Files manager methods
	void opened_files_inc();
	void opened_files_dec();
	
} // details
} // forest

#endif // FOREST_DBUTILS_H
