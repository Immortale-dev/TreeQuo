#include "tree_owner.hpp"

forest::details::tree_owner::tree_owner(tree_ptr tree) : tree(tree)
{
	// No need to reserve as tree is reserved already
}

forest::details::tree_owner::~tree_owner()
{
	cache::tree_lock();
	tree->tree_release();
	cache::tree_unlock();
}

forest::details::string forest::details::tree_owner::get_annotation()
{
	return tree->get_annotation();
}

forest::TREE_TYPES forest::details::tree_owner::get_type()
{
	return tree->get_type();
}

forest::details::tree_ptr forest::details::tree_owner::get_tree()
{
	return tree;
}

forest::details::tree_ptr forest::details::extract_native_tree(tree_owner_ptr t)
{
	return t->get_tree();
}
