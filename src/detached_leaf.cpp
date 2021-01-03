#include "detached_leaf.hpp"


forest::details::detached_leaf::detached_leaf(file_data_ptr data) : data(data)
{
	// ctor
}

forest::details::detached_leaf::~detached_leaf()
{
	// dtor
}

forest::details::file_data_t::file_data_reader forest::details::detached_leaf::get_reader()
{
	return data->get_reader();
}

forest::details::file_data_ptr forest::details::detached_leaf::get_data()
{
	return data;
}

forest::details::uint_t forest::details::detached_leaf::size()
{
	return data->size();
}

forest::details::file_data_ptr forest::details::extract_leaf_val(detached_leaf_ptr dl)
{
	return dl->get_data();
}
