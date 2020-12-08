#ifndef FOREST_THREADING_H
#define FOREST_THREADING_H

#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

namespace forest{
	struct Thread_wait{
		static void inc();
		static void dec();
		static void wait();
		
		template<typename T, typename... Args>
		static void detached_run(T fn, Args... args){
			inc();
			std::thread([](T fn, Args... args){
				destruct_dec w;
				fn(args...);
			}, fn, args...).detach();
		}
		
		private:
			struct destruct_dec{
				~destruct_dec(){ Thread_wait::dec(); }
			};
		
			inline static int count = 0;
			inline static std::mutex m;
			inline static std::condition_variable cv;	
	};

	class Thread_worker{
		typedef std::function<void()> work_fn;
		typedef std::unique_lock<std::mutex> lock_t;
		
		public:
			Thread_worker();
			~Thread_worker();
			void close();
			void work(work_fn f);
			void wait();
			bool is_busy();
		
		private:
			void notify();
			lock_t get_lock();
			
			std::queue<work_fn> q;
			std::condition_variable cv;
			std::mutex m;
			bool active = true;
			bool busy = false;
			std::thread t;
	};
}

#endif // FOREST_THREADING_H
