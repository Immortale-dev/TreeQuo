#include "variables.hpp"

namespace forest{
	
	const string LEAF_NULL = "-";

	string ROOT_TREE = "_root";
	int ROOT_FACTOR = 100;
	int DEFAULT_FACTOR = 100;
	int INTR_CACHE_LENGTH = 100;
	int LEAF_CACHE_LENGTH = 100;
	int TREE_CACHE_LENGTH = 100;
	int CACHE_BYTES = 512;
	int CHUNK_SIZE = 1024;
	int OPENED_FILES_LIMIT = 200;
	int SCHEDULE_TIMER = 2500;
}
