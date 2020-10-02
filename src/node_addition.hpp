#ifndef FOREST_NODE_ADDITION
#define FOREST_NODE_ADDITION

#include <mutex>
#include <condition_variable>
#include <memory>

namespace forest{
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
		std::weak_ptr<void> original;
		bool bloomed = true;
		bool is_original = false;
	};
}

#endif //FOREST_NODE_ADDITION