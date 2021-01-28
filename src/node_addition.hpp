#ifndef FOREST_NODE_ADDITION_H
#define FOREST_NODE_ADDITION_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>
#include "dbutils.hpp"


namespace forest{
namespace details{
	
	
	class Tree;
	
	namespace cache{
		struct node_cache_ref_t;
	}

	struct node_addition{
		struct{
			std::mutex m,g;
			int c = 0;
			bool wlock = false;
		} travel_locks;
		struct{
			std::mutex m;
			int c = 0;
		} owner_locks;
		struct{
			std::mutex m,g,p;
			bool wlock = false;
			int c = 0;
			bool promote = false;
			std::condition_variable cond, p_cond;
			bool shared_lock = false;
		} change_locks;
		std::shared_ptr<void> drive_data;
		std::shared_ptr<DBFS::File> f;
		std::weak_ptr<tree_t::Node> original;
		
		cache::node_cache_ref_t* cached_ref;
		std::list<node_ptr>::iterator cache_iterator;
		bool cache_iterator_valid = false;
		
		bool bloomed = true;
		bool is_original = false;
		bool leaved = false;
	};
	
} // details
} // forest

#endif //FOREST_NODE_ADDITION_H
