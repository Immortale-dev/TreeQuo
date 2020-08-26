#include "savior.hpp"

std::string thread_id_str()
{
	std::stringstream ss;
	ss << std::this_thread::get_id();
	return ss.str();
}

namespace forest{
	uint_t SAVIOR_IDLE_TIME_MS = 50;
	uint_t SAVIOR_DEPEND_CLUSTER_LIMIT = 10;
	uint_t SAVIOR_DEPEND_CLUSTER_REDUCE_LENGTH = SAVIOR_DEPEND_CLUSTER_LIMIT * 0.75;
}

forest::Savior::Savior()
{
	//======//log_info_private("[Savior::Savior] start");
	set_cluster_limit(SAVIOR_DEPEND_CLUSTER_LIMIT);
	set_cluster_reduce_length(SAVIOR_DEPEND_CLUSTER_REDUCE_LENGTH);
	set_timer(SAVIOR_IDLE_TIME_MS);
	//======//log_info_private("[Savior::Savior] end");
}

forest::Savior::~Savior()
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//======//log_info_private("[Savior::~Savior] start");
	save_all();
	//std::cout << "SAVE_ALL_END!!!\n";
	//======//log_info_private("[Savior::~Savior] end");
}

void forest::Savior::put(save_key item, SAVE_TYPES type, void_shared node)
{
	//======//log_info_private("[Savior::put] ("+item+") start");
	//std::cout << "SAVE_PUT_START" << std::endl;
	///if(type == SAVE_TYPES::LEAF){
	///	put_leaf(item, node);
	///} else if(type == SAVE_TYPES::INTR) {
	///	put_internal(item, node);
	///} else {
	///	put_base(item, node);
	///}
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(locking_items.count(item)){
		cv.wait(lock);
	}
	
	define_item(item, type, ACTION_TYPE::SAVE, node);
	//std::cout << "SAVE_PUT_END" << std::endl;
	
	//return;
	if(type == SAVE_TYPES::INTR || type == SAVE_TYPES::BASE){
		return;
	}
	items_queue.push(item, true);
	run_scheduler();
	//======//log_info_private("[Savior::put] end");
}

void forest::Savior::remove(save_key item, SAVE_TYPES type, void_shared node)
{
	//======//log_info_private("[Savior::remove] ("+item+") start");
	//std::cout << "S_REMOVE_START\n";
	
	//===std::cout << "+REMOVE_LOCK\n";
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(locking_items.count(item)){
		cv.wait(lock);
	}
	
	//===std::cout << "-REMOVE_LOCK\n";
	define_item(item, type, ACTION_TYPE::REMOVE, node);
	//std::cout << "S_REMOVE_DEFINED\n";
	
	save_value* it = map[item];
	//save_value* it = own_item(item);
	it->action = ACTION_TYPE::REMOVE;
	//free_item(item);
	
	//return;
	if(type == SAVE_TYPES::INTR || type == SAVE_TYPES::BASE){
		return;
	}
	items_queue.push(item, true);
	run_scheduler();
	
	/*
	std::lock_guard<std::mutex> lock(save_mtx);
	
	map_mtx.lock();
	if(map.count(item)){
		it = map[item];
	}
	map_mtx.unlock();
	if(it){
		own_item(item);
		map_mtx.lock();
		map.erase(item);
		it->m.unlock();
		delete it;
		cv.notify_all();
		map_mtx.unlock();
	}
	*/
	//std::cout << "S_REMOVE_END\n";
	//======//log_info_private("[Savior::remove] end");
}

void forest::Savior::save(save_key item, bool sync, bool lock_leaf)
{
	//======//log_info_private("[Savior::save] ("+item+") start");
	//std::cout << "SAVE_SAVE_START" << std::endl;
	////////////{
	////////////	//std::cout << "+SAVE_LOCK\n";
	////////////	std::lock_guard<std::mutex> lock(mtx);
	////////////	//std::cout << "-SAVE_LOCK\n";
	////////////	save_queue.push(item);
	////////////	if(!sync && saving){
	////////////		//======//log_info_private("[Savior::save] (already saving) end");
	////////////		return;
	////////////	}
	////////////}
	////////////if(sync){
	////////////	schedule_save(true);
	////////////} else {
	////////////	std::thread t([this](){
	////////////		//cache::leaf_lock();
	////////////		//cache::intr_lock();
	////////////		schedule_save(false);
	////////////		//cache::intr_unlock();
	////////////		//cache::leaf_unlock();
	////////////	});
	////////////	t.detach();
	////////////}
	if(sync){
		save_item(item, true);
	} else {
		std::thread t([this, item](){
			save_item(item, false);
		});
		t.detach();
	}
	//std::cout << "SAVE_SAVE_END" << std::endl;
	//======//log_info_private("[Savior::save] end");
}

void forest::Savior::get(save_key item)
{
	//======//log_info_private("[Savior::get] start");
	//std::cout << "SAVE_GET_START" << std::endl;
	//===std::cout << "+GET_LOCK\n";
	std::unique_lock lock(map_mtx);
	//===std::cout << "-GET_LOCK\n";
	//if(map.count(item)){
	//	lock.unlock();
	//	save(item,true);
	//}
	while(map.count(item)){
		cv.wait(lock);
	}
	//std::cout << "SAVE_GET_END" << std::endl;
	//======//log_info_private("[Savior::get] end");
}

void forest::Savior::save_all()
{
	//======//log_info_private("[Savior::save_all] start");
	while(true){
		//===std::cout << "+SAVE_ALL_LOCK\n";
		map_mtx.lock();
		//===std::cout << "-SAVE_ALL_LOCK\n";
		if(map.size() == 0){
			map_mtx.unlock();
			//======//log_info_private("[Savior::save_all] end");
			return;
		}
		save_key item = (map.begin())->first;
		map_mtx.unlock();
		save(item, true);
	}
}

bool forest::Savior::has(save_key item)
{
	return map.count(item);
}

int forest::Savior::size()
{
	//===std::cout << "+SIZE_LOCK\n";
	std::unique_lock<std::mutex> lock(map_mtx);
	//===std::cout << "-SIZE_LOCK\n";
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

void forest::Savior::set_timer(uint_t time)
{
	time = time;
}

void forest::Savior::set_cluster_limit(uint_t limit)
{
	cluster_limit = limit;
}

void forest::Savior::set_cluster_reduce_length(uint_t length)
{
	cluster_reduce_length = length;
}



void forest::Savior::put_leaf(save_key item, void_shared node)
{	
	//======//log_info_private("[Savior::put_leaf] start");
	//std::cout << "PUT_LEAF_START\n";
	//===std::cout << "+PUT_LEAF_LOCK\n";
	std::lock_guard<std::mutex> lock(map_mtx);
	//===std::cout << "-PUT_LEAF_LOCK\n";
	define_item(item, SAVE_TYPES::LEAF, ACTION_TYPE::SAVE, node);
	//std::cout << "PUT_LEAF_END\n";
	//======//log_info_private("[Savior::put_leaf] end");
}

void forest::Savior::put_internal(save_key item, void_shared node)
{	
	//======//log_info_private("[Savior::put_internal] start");
	//===std::cout << "+PUT_INTERNAL_LOCK\n";
	std::lock_guard<std::mutex> lock(map_mtx);
	//===std::cout << "-PUT_INTERNAL_LOCK\n";
	define_item(item, SAVE_TYPES::INTR, ACTION_TYPE::SAVE, node);
	//======//log_info_private("[Savior::put_internal] end");
}

void forest::Savior::put_base(save_key item, void_shared node)
{
	//======//log_info_private("[Savior::put_base] start");
	//===std::cout << "+PUT_BASE_LOCK\n";
	std::lock_guard<std::mutex> lock(map_mtx);
	//===std::cout << "-PUT_BASE_LOCK\n";
	define_item(item, SAVE_TYPES::BASE, ACTION_TYPE::SAVE, node);
	//======//log_info_private("[Savior::put_base] end");
}


DBFS::File* forest::Savior::save_intr(node_ptr node)
{
	//======//log_info_private("[Savior::save_intr] start");
	//std::cout << "SAVE--------INTR_START\n";
	DBFS::File* f = DBFS::create();
	//std::cout << "SAVE--------INTR_FILE_CREATE\n";
	tree_intr_read_t intr_d;
	//std::cout << "SAVE--------INTR_SAVE_OBJECT_CREATED\n";
	//assert(node->size() > 0);
	intr_d.childs_type = ((node->first_child_node()->is_leaf()) ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	//std::cout << "SAVE--------INTR_CHILDS_TYPE\n";
	int c = node->get_nodes()->size();
	//std::cout << "SAVE--------INTR_START_BEFPRE_KEYS\n";
	auto* keys = new std::vector<tree_t::key_type>(node->keys_iterator(), node->keys_iterator_end());
	auto* nodes = new std::vector<string>(c);
	//std::cout << "SAVE--------INTR_START_ITERATE\n";
	
	//====//std::unordered_set<string> test_correct;
	
	for(int i=0;i<c;i++){
		node_data_ptr d = get_node_data( (*(node->get_nodes()))[i] );
		
		//====//if(test_correct.count(d->path)){
			//====//std::cout << "DUPLICATED: " + d->path + "\n";
			//====//assert(false);
		//====//}
		//====//test_correct.insert(d->path);
		
		(*nodes)[i] = d->path;
	}
	intr_d.child_keys = keys;
	intr_d.child_values = nodes;
	//std::cout << "SAVE--------INTR_WRITE\n";
	forest::Tree::write_intr(f, intr_d);
	
	f->close();
	
	//std::cout << "SAVE--------INTR_CLOSE\n";
	//======//log_info_private("[Savior::save_intr] end");
	return f;
}

DBFS::File* forest::Savior::save_leaf(node_ptr node, std::shared_ptr<DBFS::File> fp)
{
	//======//log_info_private("[Savior::save_leaf] start");
	
	tree_leaf_read_t leaf_d;
	auto* keys = new std::vector<tree_t::key_type>();
	auto* lengths = new std::vector<uint_t>();
	
	//node_ptr p_leaf = node->prev_leaf();
	//node_ptr n_leaf = node->next_leaf();
	
	//if(p_leaf){
	//	assert(has_data(p_leaf));
	//	node_data_ptr p_leaf_data = get_node_data(p_leaf);
	//	leaf_d.left_leaf = p_leaf_data->path;
	//} else {
	//	leaf_d.left_leaf = LEAF_NULL;
	//}
	//if(n_leaf){
	//	assert(has_data(n_leaf));
	//	node_data_ptr n_leaf_data = get_node_data(n_leaf);
	//	leaf_d.right_leaf = n_leaf_data->path;
	//} else {
	//	leaf_d.right_leaf = LEAF_NULL;
	//}
	
	node_data_ptr data = get_node_data(node);
	
	leaf_d.left_leaf = data->prev;
	leaf_d.right_leaf = data->next;
	
	///for(auto& it : *(node->get_childs()) ){
	///	keys->push_back(it->item->first);
	///	lengths->push_back(it->item->second->size());
	///}
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
	
	// Block any interactions with file until it is ready
	//===std::cout << "+SAVE_LEAF_LOCK\n";
	auto lock = fp->get_lock();
	//===std::cout << "-SAVE_LEAF_LOCK\n";
	///for(auto& it : *(node->get_childs()) ){
	///	forest::Tree::write_leaf_item(fp, it->item->second);
	///}
	start = childs->begin();
	while(start != childs->end()){
		forest::Tree::write_leaf_item(fp, start->data->item->second);
		start = childs->find_next(start);
	}
	
	
	fp->stream().flush();
	
	//======//log_info_private("[Savior::save_leaf] end");
	return fp.get();
}

DBFS::File* forest::Savior::save_base(tree_ptr tree)
{
	//======//log_info_private("[Savior::save_base] start");
	
	//std::cout << "-SAVE_BASE_START\n";
	DBFS::File* base_f = DBFS::create();
	tree_base_read_t base_d;
	base_d.type = tree->get_type();
	
	//std::cout << "CTYPE: " + std::to_string(((int)tree->get_type())) + "\n";
	
	//std::cout << thread_id_str() + " BEFORE_SAVE_TREE\n";
	
	base_d.count = tree->get_tree()->size();
	base_d.factor = tree->get_tree()->get_factor();

	//std::cout << thread_id_str() + " AFTER_SAVE_TREE\n";
	
	tree_t::node_ptr root_node = tree->get_tree()->get_root_pub();
	
	assert((bool)root_node);
	
	base_d.branch_type = (root_node->is_leaf() ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	//std::cout << "-SAVE_BASE_GOT_ROOT_TYPE\n";
	if(!has_data(root_node)){
		base_d.branch = LEAF_NULL;
	} else {		
		node_data_ptr base_data = get_node_data(root_node);
		base_d.branch = base_data->path;
	}
	forest::Tree::write_base(base_f, base_d);
	base_f->close();
	
	//std::cout << "-SAVE_BASE_END\n";
	//======//log_info_private("[Savior::save_base] end");
	return base_f;
}


void forest::Savior::save_item(save_key item, bool sync)
{
	//======//log_info_private("[Savior::save_item] ("+item+") start");
	//std::cout << "_SV_save_item " + item + "\n";
	//std::cout << "+SAVE_ITEM_FIRST_LOCK\n";
	
	std::unique_lock<std::mutex> lock(map_mtx);
	
	while(saving_items.count(item)){
		cv.wait(lock);
	}
	///map_mtx.lock();
	//std::cout << "-SAVE_ITEM_FIRST_LOCK\n";
	if(!map.count(item)){
		//std::cout << "CANCEL---_SV_save_item " + item + "\n";
		///map_mtx.unlock();
		cv.notify_all();
		//======//log_info_private("[Savior::save_item] (no item found) end");
		return;
	}
	
	
	//std::cout << "SAVE_ITEM---ITEM_SAVE_START\n";
	save_value* it = map[item];
	
	saving_items.insert(item);
	///map_mtx.unlock();
	lock.unlock();
	
	
	if(it->type == SAVE_TYPES::INTR){
		//std::cout << thread_id_str() + " SAVIOR_SAVE_INTR_START "+item+"\n";
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		//std::cout << "SAVE_INTR_BEFORE_LOCK\n";
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_INTR_GET_NODE "+item+"\n";
		if(!sync){
			forest::lock_write(node);
		}
		//std::cout << thread_id_str() + " SAVIOR_SAVE_INTR_LOCK "+item+"\n";
		//std::cout << "SAVE_INTR_MID_LOCK\n";
		//own_item(item);
		//std::cout << "SAVE_INTR_AFTER_LOCK\n";
		
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		
		//callback(it->node, it->type);
		
		if(it->action == ACTION_TYPE::SAVE){
			//std::cout << thread_id_str() + " SAVIOR_SAVE_INTR__SAVE "+item+"\n";
			//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_FILE\n";
			DBFS::File* f = save_intr(node);
			//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_AFTER_FILE\n";
			string new_name = f->name();
			delete f;
			
			//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_MOVE\n";
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			
			DBFS::remove(cur_name);
			DBFS::move(new_name, cur_name);
			
			//====//std::cout << "SAVE_INTR: " + cur_name + "\n";
			
			//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_END\n";
		} else { // REMOVE
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			//std::cout << "RM_FA_START\n";
			DBFS::remove(cur_name);
			
			//====//std::cout << "REMOVE_INTR: " + cur_name + "\n";
			//std::cout << "RM_FA_END\n";
		}
		
		if(!sync){
			forest::unlock_write(node);
		}
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_INTR_END "+item+"\n";
	} else if(it->type == SAVE_TYPES::LEAF){
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_LEAF_START "+item+"\n";
		
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		//std::cout << "SI - C - " + std::to_string(get_data(node).change_locks.c) + "\n";
		//std::cout << "SAVE_ITEM---BEFORE_CHANGE_LOCK\n";
		//forest::lock_write(node);
		
		assert(get_data(node).is_original);
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_LEAF_GET_NODE "+item+"\n";
		if(!sync){
			lock_write(node);
			change_lock_write(node);
		}
		//std::cout << thread_id_str() + " SAVIOR_SAVE_LEAF_LOCK "+item+"\n";
		//std::cout << "SAVE_ITEM---AFTER_CHANGE_LOCK\n";
		//own_item(item);
		//std::cout << "SAVE_ITEM---AFTER_OWN_LOCK\n";
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		//callback(it->node, it->type);
		
		if(it->action == ACTION_TYPE::SAVE){
			
			//std::cout << "SAVE_LEAF_START\n";
			
			//std::cout << thread_id_str() + " SAVIOR_SAVE_LEAF__SAVE "+item+"\n";
			
			node_data_ptr data = get_node_data(node);
			string cur_name = data->path;
			
			//====//std::cout << "SAVE_ITEM_LEAF---"+item+"\n";
			{
				std::shared_ptr<DBFS::File> cur_f = get_data(node).f;
				if(cur_f){
					//====//std::cout << "MOVE_LEAF: " + cur_name + "\n";
					//std::cout << "CACHE_FILE: " + cur_f->name() + "\n";
					//auto locked = cur_f->get_lock();
					//cur_f->close();
					//string newnamef = DBFS::random_filename();
					//DBFS::move(cur_f->name(), newnamef);
					//cur_f->open(newnamef);
					//====//std::cout << "SAVE_ITEM_LEAF_MOVE_START---"+item+"\n";
					auto locked = cur_f->get_lock();
					cur_f->move(DBFS::random_filename());
					cur_f->on_close([](DBFS::File* file){
						file->remove();
					});
					//====//std::cout << "SAVE_ITEM_LEAF_MOVE_END---"+item+"\n";
				}
			}
			std::shared_ptr<DBFS::File> fp = std::shared_ptr<DBFS::File>(DBFS::create(cur_name));
			get_data(node).f = fp;
			save_leaf(node, fp);
			
			
			//====//std::cout << "SAVE_LEAF: " + cur_name + "\n";
			if(!sync){
				cache::leaf_lock();
			}
			own_lock(node);
			if(!get_data(node).bloomed){
				fp->close();
			}
			own_unlock(node);
			if(!sync){
				cache::leaf_unlock();
			}
			// TODO: uncomment!
			//cache::leaf_lock();
			//if(!cache::leaf_cache_r.count(item) || cache::leaf_cache_r[item].first.get() != node.get()){
			//	fp->close();
			//}
			//cache::leaf_unlock();
			
			// FUCK IT!
			///////////////////////////////////////////////////////////////////
			// TODO: should be deleted when BPT::insert move semantic done   //
			// std::cout << "CLOSE_FUCKING_FILE!!! " + cur_name + "\n";      //
			// fp->close();                                                  //
			///////////////////////////////////////////////////////////////////
			
			
			//std::cout << "SAVE_LEAF_END\n";
			
		} else { // REMOVE
			//node_data_ptr data = get_node_data(node);
			//string cur_name = data->path;
			std::shared_ptr<DBFS::File> cur_f = get_data(node).f;
			//if(cur_f){
			if(cur_f){
				//====//std::cout << "PUT_ON_DIE_LEAF: " + cur_f->name() + "\n";
				cur_f->on_close([](DBFS::File* file){
					//====//std::cout << "REMOVE_LEAF: " + file->name() + "\n";
					file->remove();
				});
			}
			
			get_data(node).f = nullptr;
			//} else {
				//DBFS::remove(cur_name);
			//}
		}
		
		if(!sync){
			change_unlock_write(node);
			unlock_write(node);
		}
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_LEAF_END "+item+"\n";
		//forest::unlock_write(node);
	} else {
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_TREE_START "+item+"\n";
		//std::cout << "_SV save_tree? " + item + "\n";
		
		//std::cout << "SAVE_ITEM_BASE_START\n";
		tree_ptr tree = std::static_pointer_cast<Tree>(it->node);
		//std::cout << thread_id_str() + " SAVE_ITEM_BASE_GET_NODE\n";
		assert((bool)tree->get_tree());
		tree->get_tree()->lock_write();
		//std::cout << thread_id_str() + " SAVE_ITEM_BASE_LOCK_TREE\n";
		//own_item(item);
		//std::cout << "SAVE_ITEM_BASE_LOCK_ITEM\n";
		lock.lock();
		locking_items.insert(item);
		lock.unlock();
		//callback(it->node, it->type);
		
		if(it->action == ACTION_TYPE::SAVE){
			
			//std::cout << "_SV save_tree? REALLY_SAVE?" + item + "\n";
			//std::cout << "SAVE_ITEM_BASE_SAVE_START\n";
			DBFS::File* base_f = save_base(tree);
			//std::cout << "SAVE_ITEM_BASE_SAVE_--save-base\n";
			
			string base_file_name = tree->get_name();
			string new_base_file_name = base_f->name();
			
			//std::cout << thread_id_str() + "_SV tree original name: " + base_file_name + " ; new_name: " + new_base_file_name + " ;\n";
			//std::cout << "SAVE_ITEM_BASE_SAVE_--get-names\n";
			
			delete base_f;
			//std::cout << "SAVE_ITEM_BASE_SAVE_--delete-file\n";
			DBFS::remove(base_file_name);
			//std::cout << "SAVE_ITEM_BASE_SAVE_--remove-old-file\n";
			DBFS::move(new_base_file_name, base_file_name);
			//std::cout << "SAVE_ITEM_BASE_SAVE_END\n";
			
			//std::cout << "_SV tree SAVED TO: " + base_file_name + " ;\n";
		} else { // REMOVE
			//std::cout << thread_id_str() + "DELETE_FILE " + item + "\n";
			///string base_file_name = tree->get_name();
			//std::cout << thread_id_str() + "RM_FL_START\n";
			DBFS::remove(item);
			//std::cout << thread_id_str() + "RM_FL_END\n";
		}
		
		assert((bool)tree->get_tree());
		tree->get_tree()->unlock_write();
		
		//std::cout << thread_id_str() + " SAVIOR_SAVE_TREE_END\n";
		//std::cout << "SAVE_ITEM_BASE_END\n";
	}
	
	// Remove item
	//std::cout << "+SAVE_ITEM_SECOND_LOCK\n";
	lock.lock();
	//std::cout << "-SAVE_ITEM_SECOND_LOCK\n";
	map.erase(item);
	saving_items.erase(item);
	locking_items.erase(item);
	//std::cout << "_SV remove_item_from_map " + item + "\n"; 
	// Free item
	//it->m.unlock();
	delete it;
	cv.notify_all();
	///map_mtx.unlock();
	//std::cout << "SAVE_ITEM---ITEM_SAVE_END\n";
	//std::cout << "S_SS_LEAF_END\n";
	//======//log_info_private("[Savior::save_item] end");
}

void forest::Savior::run_scheduler()
{
	//map_mtx.lock();
	if(scheduler_running){
		//map_mtx.unlock();
		return;
	}
	
	scheduler_running = true;
	//map_mtx.unlock();
	
	std::thread t([this](){		
		delayed_save();
	});
	t.detach();
}

void forest::Savior::delayed_save()
{
	while(true){
		//std::cout << "LOOP_1\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(schedule_timer));
		
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

void forest::Savior::schedule_save(bool sync)
{
	//======//log_info_private("[Savior::schedule_save] start");
	assert(false);
	//std::cout << "+SCHEDULE_SAVE_FIRST_LOCK\n";
	mtx.lock();
	//std::cout << "-SCHEDULE_SAVE_FIRST_LOCK\n";
	saving = true;
	mtx.unlock();

	//std::cout << "+SCHEDULE_SAVE_SECOND_LOCK\n";
	///std::lock_guard<std::mutex> lock(save_mtx);
	//std::cout << "-SCHEDULE_SAVE_SECOND_LOCK\n";
	while(true){
		//std::cout << "+SCHEDULE_SAVE_THIRD_LOCK\n";
		mtx.lock();
		//std::cout << "-SCHEDULE_SAVE_THIRD_LOCK\n";
		if(save_queue.empty()){
			saving = false;
			mtx.unlock();
			break;
		}
		string item = save_queue.front();
		save_queue.pop();
		mtx.unlock();
		save_item(item, sync);
	}
	
	//======//log_info_private("[Savior::schedule_save] end");
}

forest::Savior::save_value* forest::Savior::own_item(save_key item)
{
	//======//log_info_private("[Savior::own_item] ("+item+") start");
	//===std::cout << "+OWN_ITEM_LOCK\n";
	std::unique_lock<std::mutex> lock(map_mtx);
	//===std::cout << "-OWN_ITEM_LOCK\n";
	while(map.count(item) && map[item]->using_now){
		cv.wait(lock);
	}
	//===std::cout << "+OWN_ITEM_SECOND_LOCK\n";
	map[item]->m.lock();
	//===std::cout << "-OWN_ITEM_SECOND_LOCK\n";
	map[item]->using_now = true;
	//======//log_info_private("[Savior::own_item] end");
	return map[item];
}

void forest::Savior::free_item(save_key item)
{
	//======//log_info_private("[Savior::free_item] ("+item+") start");
	//===std::cout << "+FREE_ITEM_LOCK\n";
	std::unique_lock<std::mutex> lock(map_mtx);
	//===std::cout << "-FREE_ITEM_LOCK\n";
	map[item]->using_now = false;
	map[item]->m.unlock();
	cv.notify_all();
	//======//log_info_private("[Savior::free_item] end");
}

forest::void_shared forest::Savior::define_item(save_key item, SAVE_TYPES type, ACTION_TYPE action, void_shared node)
{
	//======//log_info_private("[Savior::define_item] ("+item+") start");
	if(!map.count(item)){
		save_value* val = new save_value();
		map[item] = val;
		val->action = action;
		val->type = type;
		val->node = node;
	}
	//======//log_info_private("[Savior::define_item] end");
	return map[item]->node;
}
