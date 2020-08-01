#include "leaf_record.hpp"

forest::LeafRecord::LeafRecord()
{
	//std::cout << "LR_CREATE\n";
	//ctor
}

forest::LeafRecord::~LeafRecord()
{
	//std::cout << "LR_DELETE\n";
	if(tree){
		tree->tree_release();
	}
	//dtor
}

forest::LeafRecord::LeafRecord(tree_t::iterator it, tree_ptr tree) : it(std::move(it)), tree(tree)
{
	//std::cout << "LR_CREATE\n";
	//if(!eof() && tree){
		//std::cout << (it->first) << std::endl;
	tree->tree_reserve();
	//}
	//std::cout << "LR_CREA_TED\n";
	// main ctor
}

bool forest::LeafRecord::eof()
{
	return it.expired();
}

bool forest::LeafRecord::move_forward()
{
	++it;
	return !eof();
}

bool forest::LeafRecord::move_back()
{
	--it;
	return !eof();
}

forest::file_data_ptr forest::LeafRecord::val()
{
	return it->second;
}

forest::string forest::LeafRecord::key()
{
	return it->first;
}
