#ifndef FOREST_LEAF_RECORD_H
#define FOREST_LEAF_RECORD_H

#include <memory>
#include "dbutils.hpp"
#include "tree.hpp"
#include "detached_leaf.hpp"

namespace forest{
namespace details{
	
	class LeafRecord{
		
		public:
			LeafRecord();
			LeafRecord(tree_t::iterator it, tree_ptr tree);
			virtual ~LeafRecord();
			
			bool eof();
			bool move_forward();
			bool move_back();
			detached_leaf_ptr val();
			string key();
		private:
			tree_t::iterator it;
			tree_ptr tree;
	};
	
	using LeafRecord_ptr = std::shared_ptr<LeafRecord>;
	
} // details
} // forest

#endif // FOREST_LEAF_RECORD_H
