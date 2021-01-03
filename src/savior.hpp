#ifndef FOREST_SAVIOR_H
#define FOREST_SAVIOR_H

#include <queue>
#include <thread>
#include "dbutils.hpp"
#include "node_data.hpp"
#include "lock.hpp"
#include "cache.hpp"
#include "tree.hpp"
#include "listcache.hpp"

namespace forest{
namespace details{
	
	extern std::atomic<int> opened_files_count;
	extern std::mutex opened_files_m;
	extern std::condition_variable opened_files_cv;
	
	extern int SCHEDULE_TIMER;
	extern int SAVIOUR_QUEUE_LENGTH;
	
	class Savior{
		
		enum class ACTION_TYPE{ SAVE, REMOVE };
		
		struct save_value{
			void_shared node;
			SAVE_TYPES type;
			ACTION_TYPE action;
			bool using_now = false;
		};
		
		public:
			using save_key = string;
			using callback_t = std::function<void(void_shared, SAVE_TYPES)>;
			
			Savior();
			virtual ~Savior();
			void put(save_key item, SAVE_TYPES type, void_shared node);
			void remove(save_key item, SAVE_TYPES type, void_shared node);
			void leave(save_key item, SAVE_TYPES type, void_shared node);
			void save(save_key item, bool async = false);
			void get(save_key item);
			int save_queue_size();
			
		private:
			void save_item(save_key item);
			save_value* define_item(save_key item, SAVE_TYPES type, ACTION_TYPE action, void_shared node);
			void run_scheduler();
			void delayed_save();
			void schedule_save(save_key& item);
			void remove_file_async(string name);
			save_value* get_item(save_key& item);
			save_value* lock_item(save_key& item);
			void pop_item(save_key& item);
			void lazy_delete_file(file_ptr f);
			bool has(save_key& item);
			bool has_locking(save_key& item);
			void lock_map();
			void unlock_map();
			void wait_for_threads();
			void save_all();
			
			Thread_worker scheduler_worker;
			Thread_worker joiner;
			Thread_worker file_deleter;
			
			callback_t callback;
		
			std::mutex map_mtx, join_mtx;
			std::condition_variable cv;
			
			uint_t time;
			uint_t cluster_limit;
			uint_t cluster_reduce_length;
			std::unordered_map<save_key, std::queue<save_value*>> map;
			std::unordered_set<save_key> saving_items, locking_items;
			bool saving = false;
			bool resolving = false;
			
			ListCache<save_key, bool> items_queue;
			bool scheduler_running = false;
			
			std::queue<std::thread> thrds;
	};
	
} // details
} // forest


#endif // FOREST_SAVIOR_H
