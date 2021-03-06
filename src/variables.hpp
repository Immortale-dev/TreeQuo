#ifndef FOREST_VARIABLES_H
#define FOREST_VARIABLES_H

#include "dbutils.hpp"

namespace forest{
namespace details{
	
	// Variables
	extern tree_ptr FOREST;
	extern bool blossomed;
	extern bool folding;
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
	extern int CHUNK_SIZE;
	extern int OPENED_FILES_LIMIT;
	extern int SAVIOUR_QUEUE_LENGTH;
	
} // details
} // forest

#endif // FOREST_VARIABLES_H
