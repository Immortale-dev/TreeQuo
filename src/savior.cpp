#include "savior.hpp"

namespace forest{
	std::atomic<int> opened_files_count = 0;
	std::mutex opened_files_m;
	std::condition_variable opened_files_cv;
}

forest::Savior::Savior()
{

}

forest::Savior::~Savior()
{
	save_all();
	
	// Wait for threads to finish
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void forest::Savior::put(save_key item, SAVE_TYPES type, void_shared node)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(locking_items.count(item)){
		cv.wait(lock);
	}
	
	define_item(item, type, ACTION_TYPE::SAVE, node);
	
	items_queue.push(item, true);
	run_scheduler();
}

void forest::Savior::remove(save_key item, SAVE_TYPES type, void_shared node)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(locking_items.count(item)){
		cv.wait(lock);
	}
	
	define_item(item, type, ACTION_TYPE::REMOVE, node);
	
	save_value* it = map[item];
	it->action = ACTION_TYPE::REMOVE;
	
	items_queue.push(item, true);
	run_scheduler();
}

void forest::Savior::leave(save_key item, SAVE_TYPES type, void_shared nodef)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	
	if(!map.count(item)){
		if(type == SAVE_TYPES::LEAF){
			node_ptr node = std::static_pointer_cast<tree_t::Node>(nodef);
			auto f = get_data(node).f;
			if(f){
				f->close();
				get_data(node).f = nullptr;
			}
		}
	}
}

void forest::Savior::save(save_key item, bool sync)
{
	if(sync){
		save_item(item);
	} else {
		std::thread t([this, item](){
			save_item(item);
		});
		t.detach();
	}
}

void forest::Savior::get(save_key item)
{
	std::unique_lock lock(map_mtx);
	while(map.count(item)){
		cv.wait(lock);
	}
}

void forest::Savior::save_all()
{
	while(true){
		std::unique_lock<std::mutex> lock(map_mtx);
		if(map.size() == 0){
			while(items_queue.size()){
				cv.wait(lock);
			}
			return;
		}
		save_key item = (map.begin())->first;
		lock.unlock();
		save(item, true);
	}
}

bool forest::Savior::has(save_key item)
{
	return map.count(item);
}

int forest::Savior::size()
{
	std::unique_lock<std::mutex> lock(map_mtx);
	return map.size();
}

void forest::Savior::lock_map()
{
	map_mtx.lock();
}

void forest::Savior::unlock_map()
{
	map_mtx.unlock();
}



void forest::Savior::put_leaf(save_key item, void_shared node)
{	
	std::lock_guard<std::mutex> lock(map_mtx);
	define_item(item, SAVE_TYPES::LEAF, ACTION_TYPE::SAVE, node);
}

void forest::Savior::put_internal(save_key item, void_shared node)
{	
	std::lock_guard<std::mutex> lock(map_mtx);
	define_item(item, SAVE_TYPES::INTR, ACTION_TYPE::SAVE, node);
}

void forest::Savior::put_base(save_key item, void_shared node)
{
	std::lock_guard<std::mutex> lock(map_mtx);
	define_item(item, SAVE_TYPES::BASE, ACTION_TYPE::SAVE, node);
}


DBFS::File* forest::Savior::save_intr(node_ptr node)
{
	DBFS::File* f = DBFS::create();
	tree_intr_read_t intr_d;
	intr_d.childs_type = ((node->first_child_node()->is_leaf()) ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	int c = node->get_nodes()->size();
	auto* keys = new std::vector<tree_t::key_type>(node->keys_iterator(), node->keys_iterator_end());
	auto* nodes = new std::vector<string>(c);
	
	for(int i=0;i<c;i++){
		node_data_ptr d = get_node_data( (*(node->get_nodes()))[i] );
		(*nodes)[i] = d->path;
	}
	intr_d.child_keys = keys;
	intr_d.child_values = nodes;
	forest::Tree::write_intr(f, intr_d);
	
	f->close();
	return f;
}

DBFS::File* forest::Savior::save_leaf(node_ptr node, std::shared_ptr<DBFS::File> fp)
{	
	tree_leaf_read_t leaf_d;
	auto* keys = new std::vector<tree_t::key_type>();
	auto* lengths = new std::vector<uint_t>();
	
	node_data_ptr data = get_node_data(node);
	
	leaf_d.left_leaf = data->prev;
	leaf_d.right_leaf = data->next;
	
	auto* childs = node->get_childs();
	tree_t::childs_type_iterator start;
	
	start = childs->begin();
	while(start != childs->end()){
		keys->push_back(start->data->item->first);
		lengths->push_back(start->data->item->second->size());
		start = childs->find_next(start);
	}
	
	leaf_d.child_keys = keys;
	leaf_d.child_lengths = lengths;
	forest::Tree::write_leaf(fp, leaf_d);
	
	auto lock = fp->get_lock();
	
	start = childs->begin();
	while(start != childs->end()){
		forest::Tree::write_leaf_item(fp, start->data->item->second);
		start = childs->find_next(start);
	}
	
	fp->stream().flush();
	
	return fp.get();
}

DBFS::File* forest::Savior::save_base(tree_ptr tree)
{
	DBFS::File* base_f = DBFS::create();
	tree_base_read_t base_d;
	base_d.type = tree->get_type();
	
	base_d.count = tree->get_tree()->size();
	base_d.factor = tree->get_tree()->get_factor();
	
	tree_t::node_ptr root_node = tree->get_tree()->get_root_pub();
	
	base_d.branch_type = (root_node->is_leaf() ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	if(!has_data(root_node)){
		base_d.branch = LEAF_NULL;
	} else {		
		node_data_ptr base_data = get_node_data(root_node);
		base_d.branch = base_data->path;
	}
	
	base_d.annotation = tree->annotation;
	
	forest::Tree::write_base(base_f, base_d);
	base_f->close();
	
	return base_f;
}


void forest::Savior::save_item(save_key item)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(saving_items.count(item)){
		cv.wait(lock);
	}
	if(!map.count(item)){
		cv.notify_all();
		return;
	}
	
	save_value* it = map[item];
	
	saving_items.insert(item);
	lock.unlock();
	
	if(it->type == SAVE_TYPES::INTR){
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		forest::lock_write(node);
		
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		
		if(it->action == ACTION_TYPE::SAVE){
			DBFS::File* f = save_intr(node);
			string new_name = f->name();
			delete f;
			
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			
			DBFS::remove(cur_name);
			DBFS::move(new_name, cur_name);
		} else { // REMOVE
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			DBFS::remove(cur_name);
		}
		
		forest::unlock_write(node);
	} else if(it->type == SAVE_TYPES::LEAF){
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		change_lock_write(node);
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		
		if(it->action == ACTION_TYPE::SAVE){
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			
			{
				std::shared_ptr<DBFS::File> cur_f = get_data(node).f;
				if(cur_f){
					// preserve limit
					std::unique_lock<std::mutex> flock(forest::opened_files_m);
					while(forest::opened_files_count > forest::OPENED_FILES_LIMIT){
						forest::opened_files_cv.wait(flock);
					}
					forest::opened_files_count++;
					flock.unlock();
					
					auto locked = cur_f->get_lock();
					cur_f->move(DBFS::random_filename());
					cur_f->on_close([](DBFS::File* file){
						// preserve limit
						forest::opened_files_count--;
						forest::opened_files_cv.notify_all();
						file->remove();
					});
				}
			}
			
			std::shared_ptr<DBFS::File> fp = std::shared_ptr<DBFS::File>(Tree::create_leaf_file(cur_name));
			get_data(node).f = fp;
			save_leaf(node, fp);
		} else { // REMOVE
			std::shared_ptr<DBFS::File> cur_f = get_data(node).f;
			if(cur_f){
				std::unique_lock<std::mutex> flock(forest::opened_files_m);
				while(forest::opened_files_count > forest::OPENED_FILES_LIMIT){
					forest::opened_files_cv.wait(flock);
				}
				forest::opened_files_count++;
				flock.unlock();
				cur_f->on_close([](DBFS::File* file){
					forest::opened_files_count--;
					forest::opened_files_cv.notify_all();
					file->remove();
				});
			}
			get_data(node).f = nullptr;
		}
		
		change_unlock_write(node);
	} else {
		tree_ptr tree = std::static_pointer_cast<Tree>(it->node);
		tree->get_tree()->lock_write();
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		
		if(it->action == ACTION_TYPE::SAVE){
			
			// Lock root node before accessing
			node_ptr root_node = tree->get_tree()->get_root_pub();
			lock_write(root_node);
			change_lock_write(root_node);
			DBFS::File* base_f = save_base(tree);
			change_unlock_write(root_node);
			unlock_write(root_node);
			
			string base_file_name = tree->get_name();
			string new_base_file_name = base_f->name();
			
			delete base_f;
			DBFS::remove(base_file_name);
			DBFS::move(new_base_file_name, base_file_name);
		} else { // REMOVE
			DBFS::remove(item);
		}
		tree->get_tree()->unlock_write();
	}
	
	// Remove item
	lock.lock();
	
	if(it->type == SAVE_TYPES::LEAF){
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		auto& data = get_data(node);
		if(!data.bloomed && data.f){
			data.f->close();
			data.f = nullptr;
		}
	}
	
	map.erase(item);
	saving_items.erase(item);
	locking_items.erase(item);
	items_queue.remove(item);
	// Free item
	delete it;
	cv.notify_all();
}

void forest::Savior::run_scheduler()
{
	if(scheduler_running){
		return;
	}
	
	scheduler_running = true;
	
	std::thread t([this](){		
		delayed_save();
	});
	t.detach();
}

void forest::Savior::delayed_save()
{
	while(true){
		std::this_thread::sleep_for(std::chrono::microseconds(SCHEDULE_TIMER));
		
		map_mtx.lock();
		if(!items_queue.size()){
			scheduler_running = false;
			map_mtx.unlock();
			return;
		}
		///{
		save_key item = items_queue.back().first;
		items_queue.pop();
		///}
		map_mtx.unlock();
		save(item, false);
	}
}

forest::void_shared forest::Savior::define_item(save_key item, SAVE_TYPES type, ACTION_TYPE action, void_shared node)
{
	if(!map.count(item)){
		save_value* val = new save_value();
		map[item] = val;
		val->action = action;
		val->type = type;
		val->node = node;
	}
	return map[item]->node;
}
