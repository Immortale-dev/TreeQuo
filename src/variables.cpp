#include "variables.hpp"

namespace forest{
namespace details{
	
	const string LEAF_NULL = "-";

	string ROOT_TREE = "_root";
	int ROOT_FACTOR = 10;
	int DEFAULT_FACTOR = 10;
	int INTR_CACHE_LENGTH = 10;
	int LEAF_CACHE_LENGTH = 10;
	int TREE_CACHE_LENGTH = 10;
	int CACHE_BYTES = 128;
	int CHUNK_SIZE = 512;
	int OPENED_FILES_LIMIT = 20;
	int SCHEDULE_TIMER = 10000;
	int SAVIOUR_QUEUE_LENGTH = 20;
	
} // details
} // forest
