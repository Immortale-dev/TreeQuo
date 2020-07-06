#include "savior.hpp"

namespace forest{
	uint_t SAVIOR_IDLE_TIME_MS = 50;
	uint_t SAVIOR_DEPEND_CLUSTER_LIMIT = 10;
	uint_t SAVIOR_DEPEND_CLUSTER_REDUCE_LENGTH = SAVIOR_DEPEND_CLUSTER_LIMIT * 0.75;
}

forest::Savior::Savior()
{
	set_cluster_limit(SAVIOR_DEPEND_CLUSTER_LIMIT);
	set_cluster_reduce_length(SAVIOR_DEPEND_CLUSTER_REDUCE_LENGTH);
	set_timer(SAVIOR_IDLE_TIME_MS);
}

forest::Savior::~Savior()
{
	save_all();
}

void forest::Savior::put(save_key item, SAVE_TYPES type)
{
	//std::cout << "SAVE_PUT_START" << std::endl;
	if(type == SAVE_TYPES::LEAF){
		put_leaf(item);
	} else if(type == SAVE_TYPES::INTR) {
		put_internal(item);
	} else {
		put_base(item);
	}
	resolve_cluster();
	//std::cout << "SAVE_PUT_END" << std::endl;
}

void forest::Savior::remove(save_key item)
{
	//std::cout << "S_REMOVE_START\n";
	std::lock_guard<std::mutex> lock(save_mtx);
	save_value* it = nullptr;
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
	//std::cout << "S_REMOVE_END\n";
}

void forest::Savior::save(save_key item)
{
	//std::cout << "SAVE_SAVE_START" << std::endl;
	{
		std::lock_guard<std::mutex> lock(mtx);
		save_queue.push(item);
	}
	schedule_save();
	//std::cout << "SAVE_SAVE_END" << std::endl;
}

void forest::Savior::get(save_key item)
{
	//std::cout << "SAVE_GET_START" << std::endl;
	std::unique_lock lock(map_mtx);
	while(map.count(item)){
		cv.wait(lock);
	}
	//std::cout << "SAVE_GET_END" << std::endl;
}

void forest::Savior::save_all()
{
	while(true){
		map_mtx.lock();
		if(map.size() == 0){
			map_mtx.unlock();
			return;
		}
		save_key item = (map.begin())->first;
		map_mtx.unlock();
		save(item);
	}
}

int forest::Savior::size()
{
	std::unique_lock<std::mutex> lock(map_mtx);
	return map.size();
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



void forest::Savior::put_leaf(save_key item)
{	
	define_item(item, SAVE_TYPES::LEAF);
}

void forest::Savior::put_internal(save_key item)
{	
	define_item(item, SAVE_TYPES::INTR);
}

void forest::Savior::put_base(save_key item)
{
	define_item(item, SAVE_TYPES::BASE);
}


DBFS::File* forest::Savior::save_intr(node_ptr node)
{
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
	for(int i=0;i<c;i++){
		node_data_ptr d = get_node_data( (*(node->get_nodes()))[i] );
		(*nodes)[i] = d->path;
	}
	intr_d.child_keys = keys;
	intr_d.child_values = nodes;
	//std::cout << "SAVE--------INTR_WRITE\n";
	forest::Tree::write_intr(f, intr_d);
	
	f->close();
	
	//std::cout << "SAVE--------INTR_CLOSE\n";
	return f;
}

DBFS::File* forest::Savior::save_leaf(node_ptr node, std::shared_ptr<DBFS::File> fp)
{
	tree_leaf_read_t leaf_d;
	auto* keys = new std::vector<tree_t::key_type>();
	auto* lengths = new std::vector<uint_t>();
	
	node_ptr p_leaf = node->prev_leaf();
	node_ptr n_leaf = node->next_leaf();
	
	if(p_leaf){
		assert(has_data(p_leaf));
		node_data_ptr p_leaf_data = get_node_data(p_leaf);
		leaf_d.left_leaf = p_leaf_data->path;
	} else {
		leaf_d.left_leaf = LEAF_NULL;
	}
	if(n_leaf){
		assert(has_data(n_leaf));
		node_data_ptr n_leaf_data = get_node_data(n_leaf);
		leaf_d.right_leaf = n_leaf_data->path;
	} else {
		leaf_d.right_leaf = LEAF_NULL;
	}
	
	for(auto& it : *(node->get_childs()) ){
		keys->push_back(it->item->first);
		lengths->push_back(it->item->second->size());
	}
	
	leaf_d.child_keys = keys;
	leaf_d.child_lengths = lengths;
	forest::Tree::write_leaf(fp, leaf_d);
	
	// Block any interactions with file until it is ready
	auto lock = fp->get_lock();
	for(auto& it : *(node->get_childs()) ){
		forest::Tree::write_leaf_item(fp, it->item->second);
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
	node_data_ptr base_data = get_node_data(root_node);
	base_d.branch = base_data->path;
	forest::Tree::write_base(base_f, base_d);
	
	base_f->close();
	
	return base_f;
}


void forest::Savior::save_item(save_key item)
{
	map_mtx.lock();
	if(!map.count(item)){
		map_mtx.unlock();
		return;
	}
	//std::cout << "SAVE_ITEM---ITEM_SAVE_START\n";
	save_value* it = map[item];
	map_mtx.unlock();
	
	
	if(it->type == SAVE_TYPES::INTR){
		//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_START\n";
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		change_lock_read(node);
		own_item(item);
		
		//callback(it->node, it->type);
		
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
		
		//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_UNLOCK\n";
		change_unlock_read(node);
		//std::cout << "SAVE_ITEM---ITEM_SAVE_INTR_END\n";
	} else if(it->type == SAVE_TYPES::LEAF){
		node_ptr node = std::static_pointer_cast<tree_t::Node>(it->node);
		change_lock_write(node);
		own_item(item);
		
		//callback(it->node, it->type);
		
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		
		{
			DBFS::File* cur_f = get_data(node).f;
			auto locked = cur_f->get_lock();
			cur_f->move(DBFS::random_filename());
			cur_f->on_close([](DBFS::File* file){
				file->remove();
			});
		}
		std::shared_ptr<DBFS::File> fp = std::shared_ptr<DBFS::File>(new DBFS::File(cur_name));
		save_leaf(node, fp);
		
		
		change_unlock_write(node);
	} else {
		tree_ptr tree = std::static_pointer_cast<Tree>(it->node);
		tree->get_tree()->lock_read();
		own_item(item);
		
		//callback(it->node, it->type);
		
		
		DBFS::File* base_f = save_base(tree);
		
		string base_file_name = tree->get_name();
		string new_base_file_name = base_f->name();
		
		delete base_f;
		DBFS::remove(base_file_name);
		DBFS::move(new_base_file_name, base_file_name);
		
		
		tree->get_tree()->unlock_read();
	}
	
	// Remove item
	map_mtx.lock();
	map.erase(item);
	it->m.unlock();
	delete it;
	cv.notify_all();
	map_mtx.unlock();
	//std::cout << "SAVE_ITEM---ITEM_SAVE_END\n";
}

void forest::Savior::resolve_cluster()
{
	map_mtx.lock();
	if(map.size() >= cluster_limit){
		map_mtx.unlock();
		while(true){
			map_mtx.lock();
			if(map.size() <= cluster_reduce_length){
				//map_mtx.unlock();
				break;
			}
			string item = (*map.begin()).first;
			map_mtx.unlock();
			save(item);
		}
	} 
	map_mtx.unlock();
}

void forest::Savior::schedule_save()
{
	std::thread t([this](){
		//std::cout << "SAVE_ITEM_START" << std::endl;
		std::lock_guard<std::mutex> lock(save_mtx);
		while(true){
			mtx.lock();
			if(save_queue.empty()){
				mtx.unlock();
				break;
			}
			string item = save_queue.front();
			save_queue.pop();
			mtx.unlock();
			save_item(item);
		}
		//std::cout << "SAVE_ITEM_END" << std::endl;
	});
	t.detach();
}

void forest::Savior::own_item(save_key item)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	while(map.count(item) && map[item]->using_now){
		cv.wait(lock);
	}
	map[item]->m.lock();
	map[item]->using_now = true;
}

void forest::Savior::free_item(save_key item)
{
	std::unique_lock<std::mutex> lock(map_mtx);
	map[item]->using_now = false;
	map[item]->m.unlock();
	cv.notify_all();
}

forest::void_shared forest::Savior::define_item(save_key item, SAVE_TYPES type)
{
	std::lock_guard<std::mutex> lock(map_mtx);
	if(!map.count(item)){
		save_value* val = new save_value();
		map[item] = val;
		val->type = type;
		if(type == SAVE_TYPES::LEAF){
			cache::leaf_lock();
			/// lock{
			assert(cache::leaf_cache_r.count(item));
			assert(cache::leaf_cache_r[item].second > 0);
			val->node = cache::leaf_cache_r[item].first;
			/// }lock
			cache::leaf_unlock();
		} else if(type == SAVE_TYPES::INTR) {
			cache::intr_lock();
			/// lock{
			assert(cache::intr_cache_r.count(item));
			assert(cache::intr_cache_r[item].second > 0);
			val->node = cache::intr_cache_r[item].first;
			/// }lock
			cache::intr_unlock();
		} else {
			cache::tree_cache_m.lock();
			/// lock{
			assert(cache::tree_cache_r.count(item));
			assert(cache::tree_cache_r[item].second > 0);
			val->node = cache::tree_cache_r[item].first;
			/// }lock
			cache::tree_cache_m.unlock();
		}
	}
	return map[item]->node;
}
