#include "leaf_record.hpp"

forest::details::LeafRecord::LeafRecord()
{
	//ctor
}

forest::details::LeafRecord::~LeafRecord()
{
	if(tree){
		tree->tree_release();
	}
	//dtor
}

forest::details::LeafRecord::LeafRecord(tree_t::iterator it, tree_ptr tree) : it(std::move(it)), tree(tree)
{
	tree->tree_reserve();
	// main ctor
}

bool forest::details::LeafRecord::eof()
{
	return it.expired();
}

bool forest::details::LeafRecord::move_forward()
{
	++it;
	return !eof();
}

bool forest::details::LeafRecord::move_back()
{
	--it;
	return !eof();
}

forest::details::detached_leaf_ptr forest::details::LeafRecord::val()
{
	if(eof()){
		throw TreeException(TreeException::ERRORS::ACCESSING_END_LEAF);
	}
	return detached_leaf_ptr(new detached_leaf(it->second));
}

forest::details::string forest::details::LeafRecord::key()
{
	if(eof()){
		throw TreeException(TreeException::ERRORS::ACCESSING_END_LEAF);
	}
	return it->first;
}
