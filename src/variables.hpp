#ifndef FOREST_VARIABLES
#define FOREST_VARIABLES

#include "dbutils.hpp"

namespace forest{
	// Variables
	extern tree_ptr FOREST;
	extern bool blossomed;
	extern int DEFAULT_FACTOR;
	extern int INTR_CACHE_LENGTH;
	extern int LEAF_CACHE_LENGTH;
	extern int TREE_CACHE_LENGTH;
	extern string ROOT_TREE;
	extern int ROOT_FACTOR;
	extern const string LEAF_NULL;
	extern int LOGGER_FLAG;
	extern int LOG_DETAILS;
	extern int CACHE_BYTES;
}

#endif // FOREST_VARIABLES
