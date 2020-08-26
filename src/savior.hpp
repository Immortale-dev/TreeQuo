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
	
	extern uint_t SAVIOR_IDLE_TIME_MS;
	extern uint_t SAVIOR_DEPEND_CLUSTER_LIMIT;
	extern uint_t SAVIOR_DEPEND_CLUSTER_REDUCE_LENGTH;
	
	class Savior{
		
		enum class ACTION_TYPE{ SAVE, REMOVE };
		
		struct save_value{
			void_shared node;
			SAVE_TYPES type;
			ACTION_TYPE action;
			bool using_now = false;
			std::mutex m;
		};
		
		public:
			using save_key = string;
			using callback_t = std::function<void(void_shared, SAVE_TYPES)>;
			
			Savior();
			virtual ~Savior();
			void put(save_key item, SAVE_TYPES type, void_shared node);
			void remove(save_key item, SAVE_TYPES type, void_shared node);
			void save(save_key item, bool async = false, bool lock_leaf = true);
			void get(save_key item);
			void save_all();
			bool has(save_key item);
			int size();
			
			void lock_map();
			void unlock_map();
			
			void set_timer(uint_t time);
			void set_cluster_limit(uint_t limit);
			void set_cluster_reduce_length(uint_t length);
			
		private:
			void put_leaf(save_key item, void_shared node);
			void put_internal(save_key item, void_shared node);
			void put_base(save_key item, void_shared node);
			void save_item(save_key item, bool sync = false);
			void_shared define_item(save_key item, SAVE_TYPES type, ACTION_TYPE action, void_shared node);
			save_value* own_item(save_key item);
			void free_item(save_key item);
			void schedule_save(bool sync);
			void remove_item(save_key item);
			void run_scheduler();
			void delayed_save();
			DBFS::File* save_intr(node_ptr node);
			DBFS::File* save_leaf(node_ptr node, std::shared_ptr<DBFS::File> fp);
			DBFS::File* save_base(tree_ptr tree);
			
			callback_t callback;
		
			std::mutex mtx, map_mtx, save_mtx;
			std::condition_variable cv;
			
			uint_t time;
			uint_t cluster_limit;
			uint_t cluster_reduce_length;
			std::unordered_map<save_key, save_value*> map;
			std::unordered_set<save_key> saving_items, locking_items;
			std::queue<save_key> save_queue;
			bool saving = false;
			bool resolving = false;
			
			ListCache<save_key, bool> items_queue;
			bool scheduler_running = false;
			int schedule_timer = 1;
	};
}


#endif // FOREST_SAVIOR_H



