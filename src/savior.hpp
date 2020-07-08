#ifndef FOREST_SAVIOR_H
#define FOREST_SAVIOR_H

#include <queue>
#include "dbutils.hpp"
#include "node_data.hpp"
#include "lock.hpp"
#include "cache.hpp"
#include "tree.hpp"

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
			void put(save_key item, SAVE_TYPES type);
			void remove(save_key item, SAVE_TYPES type);
			void save(save_key item, bool async = false);
			void get(save_key item);
			void save_all();
			int size();
			
			void set_timer(uint_t time);
			void set_cluster_limit(uint_t limit);
			void set_cluster_reduce_length(uint_t length);
			
		private:
			void put_leaf(save_key item);
			void put_internal(save_key item);
			void put_base(save_key item);
			void save_item(save_key item);
			void_shared define_item(save_key item, SAVE_TYPES type, ACTION_TYPE action);
			save_value* own_item(save_key item);
			void free_item(save_key item);
			void schedule_save();
			void remove_item(save_key item);
			void resolve_cluster();
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
			std::queue<save_key> save_queue;
			bool saving = false;
			bool resolving = false;
	};
}


#endif // FOREST_SAVIOR_H



