#include "node_data.hpp"

forest::details::node_data_ptr forest::details::create_node_data(bool ghost, string path)
{
	return node_data_ptr(new node_data_t(ghost, path));
}

forest::details::node_data_ptr forest::details::create_node_data(bool ghost, string path, string prev, string next)
{
	auto p = node_data_ptr(new node_data_t(ghost, path));
	p->prev = prev;
	p->next = next;
	return p;
}

forest::details::node_data_ptr forest::details::get_node_data(tree_t::node_ptr node)
{
	return std::static_pointer_cast<node_data_t>(get_data(node).drive_data);
}

void forest::details::set_node_data(tree_t::node_ptr node, node_data_ptr d)
{
	set_node_data(node.get(), d);
}

void forest::details::set_node_data(tree_t::Node* node, node_data_ptr d)
{
	get_data(node).drive_data = d;
}

bool forest::details::has_data(tree_t::node_ptr node)
{
	return has_data(node.get());
}

bool forest::details::has_data(tree_t::Node* node)
{
	return (bool)(get_data(node).drive_data);
}
