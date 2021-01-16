#ifndef FOREST_TREE_OWNER_H
#define FOREST_TREE_OWNER_H

#include "dbutils.hpp"
#include "tree.hpp"

namespace forest{
namespace details{
	
	class tree_owner{
		
		friend tree_ptr extract_native_tree(tree_owner_ptr);
		
		public:
			tree_owner(tree_ptr tree);
			virtual ~tree_owner();

			string get_annotation();
			TREE_TYPES get_type();
			
		private:
			tree_ptr get_tree();
			
			tree_ptr tree;
	};
	
	tree_ptr extract_native_tree(tree_owner_ptr t);
	
} // details
} // forest

#endif // FOREST_TREE_OWNER_H
