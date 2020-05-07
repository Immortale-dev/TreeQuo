#ifndef FOREST_LEAF_ITEM
#define FOREST_LEAF_ITEM

#include <memory>
#include "dbutils.hpp"

namespace forest{
	
	class LeafRecord{
		
		public:
			LeafRecord();
			LeafRecord(tree_t::iterator it);
			virtual ~LeafRecord();
			
			bool eof();
			bool move_forward();
			bool move_back();
			file_data_ptr val();
			string key();
		private:
			tree_t::iterator it;
	};
	
	using LeafRecord_ptr = std::shared_ptr<LeafRecord>;
}

#endif // FOREST_LEAF_ITEM
