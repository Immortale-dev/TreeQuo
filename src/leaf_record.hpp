#ifndef FOREST_LEAF_ITEM
#define FOREST_LEAF_ITEM

#include <memory>
#include "dbutils.hpp"
#include "tree.hpp"

namespace forest{
	
	class LeafRecord{
		
		public:
			LeafRecord();
			LeafRecord(tree_t::iterator it, tree_ptr tree);
			virtual ~LeafRecord();
			
			bool eof();
			bool move_forward();
			bool move_back();
			file_data_ptr val();
			string key();
		private:
			tree_t::iterator it;
			tree_ptr tree;
	};
	
	using LeafRecord_ptr = std::shared_ptr<LeafRecord>;
}

#endif // FOREST_LEAF_ITEM
