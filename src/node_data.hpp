#ifndef FOREST_NODE_DATA_H
#define FOREST_NODE_DATA_H

#include "dbutils.hpp"

namespace forest{
namespace details{
	
	struct node_data_t {
		bool ghost = true;
		string path = "";
		string prev = LEAF_NULL;
		string next = LEAF_NULL;
		node_data_t(bool ghost, string path) : ghost(ghost), path(path) {};
	};
	
	using node_data_ptr = std::shared_ptr<node_data_t>;
	
	// Node data
	node_data_ptr create_node_data(bool ghost, string path);
	node_data_ptr create_node_data(bool ghost, string path, string prev, string next);
	node_data_ptr get_node_data(tree_t::node_ptr node);
	void set_node_data(tree_t::node_ptr node, node_data_ptr d);
	void set_node_data(tree_t::Node* node, node_data_ptr d);
	bool has_data(tree_t::node_ptr node);
	bool has_data(tree_t::Node* node);

} // details
} // forest

#endif // FOREST_NODE_DATA_H
