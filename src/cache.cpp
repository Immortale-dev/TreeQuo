#include "cache.hpp"

namespace forest{
	
	namespace cache{
		ListCache<string, tree_ptr> tree_cache(TREE_CACHE_LENGTH);
		ListCache<string, tree_t::node_ptr> leaf_cache(LEAF_CACHE_LENGTH), intr_cache(INTR_CACHE_LENGTH);
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		std::unordered_map<string, std::pair<tree_t::node_ptr, int_a> > intr_cache_r, leaf_cache_r;
		std::unordered_map<uintptr_t, std::unordered_map<int, int> > leaf_cache_i;
	}
	
	tree_ptr FOREST;
	bool blossomed = false;
}

/////////////CACHE_METHODS//////////////

void forest::cache::init_cache()
{
	//======//log_info_private("[cache::init_cache] start");
	tree_cache.set_callback([](string key){ forest::cache::check_tree_ref(key); });
	leaf_cache.set_callback([](string key){ forest::cache::check_leaf_ref(key); });
	intr_cache.set_callback([](string key){ forest::cache::check_intr_ref(key); });
	//======//log_info_private("[cache::init_cache] end");
}

void forest::cache::release_cache()
{
	// Remove?
}

void forest::cache::check_tree_ref(string key)
{
	//======//log_info_private("[cache::check_tree_ref] start");
	if(!tree_cache_r.count(key)){
		//======//log_info_private("[cache::check_tree_ref] (no key found) end");
		return;
	}
	if(tree_cache_r[key].second == 0 && !tree_cache.has(key)){
		savior->save(key, true);
		tree_cache_r.erase(key);
	}
	//======//log_info_private("[cache::check_tree_ref] end");
}

void forest::cache::check_leaf_ref(string key)
{
	//======//log_info_private("[cache::check_leaf_ref] start");
	if(!leaf_cache_r.count(key)){
		//======//log_info_private("[cache::check_leaf_ref] (no key found) end");
		return;
	}
	if(leaf_cache_r[key].second == 0 && !leaf_cache.has(key)){
		//====//std::cout << "LEAF_REF_START " + key + "\n";
		
		tree_t::node_ptr node = leaf_cache_r[key].first;
		////get_data(node).f = nullptr;
		////for(auto& it : (*node->get_childs())){
			////it->item->second->set_file(nullptr);
			////it->item->second = nullptr;
			//it->node = nullptr;
		////}
		node->set_next_leaf(nullptr);
		node->set_prev_leaf(nullptr);
		
		// Close file if there if it should be closed
		
		
		savior->lock_map();
		
		if(!savior->has(key) && get_data(node).f){
			 //====//std::cout << "CLOSE FILE WHICH IS NOT GOING TO BE SAVED " + key + "\n";
			get_data(node).f->close();
		}
		
		//savior->unlock_map();
		//savior->lock_map();
		
		if(savior->has(key)){
			savior->unlock_map();
			savior->save(key, true);
		} else {
			savior->unlock_map();
		}
		
		
		//savior->save(key, true);
		leaf_cache_r.erase(key);
		
		//====//std::cout << "LEAF_REF_END " + key + "\n";
	}
	//======//log_info_private("[cache::check_leaf_ref] end");
}

void forest::cache::check_intr_ref(string key)
{
	//======//log_info_private("[cache::check_intr_ref] start");
	if(!intr_cache_r.count(key)){
		//======//log_info_private("[cache::check_intr_ref] () end");
		return;
	}
	if(intr_cache_r[key].second == 0 && !intr_cache.has(key)){
		//intr_cache_r[key].first->get_nodes()->resize(0);
		
		
		savior->lock_map();
		if(savior->has(key)){
			savior->unlock_map();
			savior->save(key, true);
		} else {
			savior->unlock_map();
		}

		intr_cache_r.erase(key);
	}
	//======//log_info_private("[cache::check_intr_ref] end");
}

void forest::cache::set_tree_cache_length(int length)
{
	//======//log_info_private("[cache::set_tree_cache_length] start");
	TREE_CACHE_LENGTH = length;
	tree_cache_m.lock();
	tree_cache.resize(TREE_CACHE_LENGTH);
	tree_cache_m.unlock();
	//======//log_info_private("[cache::set_tree_cache_length] end");
}

void forest::cache::set_intr_cache_length(int length)
{
	//======//log_info_private("[cache::set_intr_cache_length] start");
	INTR_CACHE_LENGTH = length;
	intr_cache_m.lock();
	intr_cache.resize(INTR_CACHE_LENGTH);
	intr_cache_m.unlock();
	//======//log_info_private("[cache::set_intr_cache_length] end");
}

void forest::cache::set_leaf_cache_length(int length)
{
	//======//log_info_private("[cache::set_leaf_cache_length] start");
	LEAF_CACHE_LENGTH = length;
	leaf_cache_m.lock();
	leaf_cache.resize(LEAF_CACHE_LENGTH);
	leaf_cache_m.unlock();
	//======//log_info_private("[cache::set_leaf_cache_length] end");
}

void forest::cache::insert_item(uintptr_t path, int pos)
{
	//======//log_info_private("[cache::insert_item] ("+path+":"+std::to_string(pos)+") start");
	leaf_cache_i[path][pos]++;
	//======//log_info_private("[cache::insert_item] end");
}

void forest::cache::remove_item(uintptr_t path, int pos)
{
	//======//log_info_private("[cache::remove_item] ("+path+":"+std::to_string(pos)+") start");
	auto& c_node = leaf_cache_i[path];
	
	assert(c_node[pos] > 0);
	
	c_node[pos]--;
	if(!c_node[pos]){
		c_node.erase(pos);
	}
	if(!c_node.size()){
		leaf_cache_i.erase(path);
	}
	//======//log_info_private("[cache::remove_item] end");
}

void forest::cache::intr_lock()
{
	//======//log_info_private("[cache::intr_lock] start");
	intr_cache_m.lock();
	//======//log_info_private("[cache::intr_lock] end");
}

void forest::cache::intr_unlock()
{
	//======//log_info_private("[cache::intr_unlock] start");
	intr_cache_m.unlock();
	//======//log_info_private("[cache::intr_unlock] end");
}

void forest::cache::leaf_lock()
{
	//======//log_info_private("[cache::leaf_lock] start");
	leaf_cache_m.lock();
	//======//log_info_private("[cache::leaf_lock] end");
}

void forest::cache::leaf_unlock()
{
	//======//log_info_private("[cache::leaf_unlock] start");
	leaf_cache_m.unlock();
	//======//log_info_private("[cache::leaf_unlock] end");
}

std::lock_guard<std::mutex> forest::cache::get_intr_lock()
{
	return std::lock_guard<std::mutex>(intr_cache_m);
}

std::lock_guard<std::mutex> forest::cache::get_leaf_lock()
{
	return std::lock_guard<std::mutex>(leaf_cache_m);
}

void forest::cache::reserve_node(tree_t::node_ptr node, bool w_lock)
{
	//======//log_info_private("[cache::reserve_node] start");
	assert(has_data(node));
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;
	
	if(is_leaf){
		if(w_lock){
			with_lock(NODE_TYPES::LEAF, [&path](){
				reserve_leaf_node(path);
			});
		} else {
			reserve_leaf_node(path);
		}
	} else {
		if(w_lock){
			with_lock(NODE_TYPES::INTR, [&path](){
				reserve_intr_node(path);
			});
		} else {
			reserve_intr_node(path);
		}
	}
	//======//log_info_private("[cache::reserve_node] end");
}

void forest::cache::release_node(tree_t::node_ptr node, bool w_lock)
{
	//======//log_info_private("[cache::release_node] start");
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;

	if(is_leaf){
		if(w_lock){
			with_lock(NODE_TYPES::LEAF, [&path](){
				release_leaf_node(path);
			});
		} else {
			release_leaf_node(path);
		}
	} else {
		if(w_lock){
			with_lock(NODE_TYPES::INTR, [&path](){
				release_intr_node(path);
			});
		} else {
			release_intr_node(path);
		}
	}
	//======//log_info_private("[cache::release_node] end");
}

void forest::cache::with_lock(NODE_TYPES type, std::function<void()> fn)
{
	if(type == NODE_TYPES::INTR){
		intr_lock();
	} else {
		leaf_lock();
	}
	
	fn();
	
	if(type == NODE_TYPES::INTR){
		intr_unlock();
	} else {
		leaf_unlock();
	}
}

void forest::cache::reserve_intr_node(string& path)
{
	//======//log_info_private("[cache::reserve_intr_node] ("+path+") start");
	intr_cache_r[path].second++;
	//======//log_info_private("[cache::reserve_intr_node] end");
}

void forest::cache::release_intr_node(string& path)
{
	//======//log_info_private("[cache::release_intr_node] ("+path+") start");
	assert(intr_cache_r.count(path));
	assert(intr_cache_r[path].second>0);
	intr_cache_r[path].second--;
	check_intr_ref(path);
	//======//log_info_private("[cache::release_intr_node] end");
}

void forest::cache::reserve_leaf_node(string& path)
{
	//======//log_info_private("[cache::reserve_leaf_node] ("+path+") start");
	leaf_cache_r[path].second++;
	//======//log_info_private("[cache::reserve_leaf_node] end");
}

void forest::cache::release_leaf_node(string& path)
{
	//======//log_info_private("[cache::release_leaf_node] ("+path+") start");
	assert(leaf_cache_r.count(path));
	assert(leaf_cache_r[path].second>0);
	leaf_cache_r[path].second--;
	check_leaf_ref(path);
	//======//log_info_private("[cache::release_leaf_node] end");
}

void forest::cache::intr_insert(tree_t::node_ptr node, bool w_lock)
{
	//======//log_info_private("[cache::intr_insert] start");
	if(w_lock){
		with_lock(NODE_TYPES::INTR, [&node](){
			_intr_insert(node);
		});
	} else {
		_intr_insert(node);
	}
	//======//log_info_private("[cache::intr_insert] end");
}

void forest::cache::leaf_insert(tree_t::node_ptr node, bool w_lock)
{
	//======//log_info_private("[cache::leaf_insert] start");
	if(w_lock){
		with_lock(NODE_TYPES::LEAF, [&node](){
			_leaf_insert(node);
		});
	} else {
		_leaf_insert(node);
	}
	//======//log_info_private("[cache::leaf_insert] end");
}

void forest::cache::clear_node_cache(tree_t::node_ptr node)
{
	//======//log_info_private("[cache::clear_node_cache] start");
	node_data_ptr data = get_node_data(node);
	string& path = data->path;
	if(node->is_leaf()){
		cache::leaf_lock();
		if(cache::leaf_cache.has(path)){
			cache::leaf_cache.remove(path);
			//std::cout << "CACHE_CLEARED: " + path + "\n";
		}
		cache::leaf_unlock();
	}
	else{
		cache::intr_lock();
		if(cache::intr_cache.has(path)){
			cache::intr_cache.remove(path);
		}
		cache::intr_unlock();
	}
	//======//log_info_private("[cache::clear_node_cache] end");
}


void forest::cache::_intr_insert(tree_t::node_ptr node)
{
	string& path = get_node_data(node)->path;
	cache::intr_cache_r[path] = std::make_pair(node, 0);
	cache::intr_cache.push(path, node);
}

void forest::cache::_leaf_insert(tree_t::node_ptr node)
{
	string& path = get_node_data(node)->path;
	cache::leaf_cache_r[path] = std::make_pair(node, 0);
	cache::leaf_cache.push(path, node);
}
