#ifndef FOREST_DETACHED_LEAF_H
#define FOREST_DETACHED_LEAF_H

#include "dbutils.hpp"

namespace forest{
namespace details{
	
	class detached_leaf{
		
		friend file_data_ptr extract_leaf_val(detached_leaf_ptr dl);
		
		public:
			detached_leaf(file_data_ptr data);
			virtual ~detached_leaf();
			file_data_t::file_data_reader get_reader();
			uint_t size();
			
		private:
			file_data_ptr get_data();
			
			file_data_ptr data;
	};
	
	file_data_ptr extract_leaf_val(detached_leaf_ptr dl);
	
} // details
} // forest

#endif // FOREST_DETACHED_LEAF_H
