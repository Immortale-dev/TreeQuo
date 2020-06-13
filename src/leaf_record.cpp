#include "leaf_record.hpp"

forest::LeafRecord::LeafRecord()
{
	//ctor
}

forest::LeafRecord::~LeafRecord()
{
	if(tree){
		tree->tree_release();
	}
	//dtor
}

forest::LeafRecord::LeafRecord(tree_t::iterator it, tree_ptr tree) : it(std::move(it)), tree(tree)
{
	tree->tree_reserve();
	// main ctor
}

bool forest::LeafRecord::eof()
{
	return it.expired();
}

bool forest::LeafRecord::move_forward()
{
	++it;
	return eof();
}

bool forest::LeafRecord::move_back()
{
	--it;
	return eof();
}

forest::file_data_ptr forest::LeafRecord::val()
{
	return it->second;
}

forest::string forest::LeafRecord::key()
{
	return it->first;
}
