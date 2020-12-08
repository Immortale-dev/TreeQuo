#include "threading.hpp"

// Thread_wait
void forest::Thread_wait::inc(){
	std::unique_lock<std::mutex> lock(m);
	++count;
}

void forest::Thread_wait::dec(){
	std::unique_lock<std::mutex> lock(m);
	if(--count == 0){
		cv.notify_all();
	}
}

void forest::Thread_wait::wait(){
	std::unique_lock<std::mutex> lock(m);
	while (count > 0) {
		cv.wait(lock);
	}
}


// Thread_worker
forest::Thread_worker::Thread_worker(){
	t = std::thread([this]{
		while(true){
			work_fn fn;
			{
				auto lock = get_lock();
				while(q.empty()){
					busy = false;
					notify();
					if(!active) return;
					cv.wait(lock);
				}
				busy = true;
				fn = q.front();
				q.pop();
			}
			fn();
		}
	});
}

forest::Thread_worker::~Thread_worker(){ 
	close(); 
	t.join(); 
}

void forest::Thread_worker::close(){
	auto lock = get_lock();
	active = false;
	notify(); 
}

void forest::Thread_worker::work(work_fn f){
	auto lock = get_lock();
	q.push(f);
	notify();
}

void forest::Thread_worker::wait(){
	auto lock = get_lock();
	while(busy){
		cv.wait(lock);
	}
}

bool forest::Thread_worker::is_busy(){ 
	return busy; 
}

void forest::Thread_worker::notify(){ 
	cv.notify_all(); 
}

forest::Thread_worker::lock_t forest::Thread_worker::get_lock(){ 
	return lock_t(m); 
}
