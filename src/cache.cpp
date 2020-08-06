#include "cache.hpp"

extern int hook_remove_leaf;

namespace forest{
	
	namespace cache{
		ListCache<string, tree_ptr> tree_cache(TREE_CACHE_LENGTH);
		ListCache<string, tree_t::node_ptr> leaf_cache(LEAF_CACHE_LENGTH), intr_cache(INTR_CACHE_LENGTH);
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::pair<tree_ptr, int> > tree_cache_r;
		std::unordered_map<string, leaf_cache_ref_t> leaf_cache_r;
		std::unordered_map<string, std::pair<tree_t::node_ptr, int> > intr_cache_r;
		
		std::unordered_set<string> tree_cache_q;
		std::condition_variable tree_cv;
		std::mutex tree_cv_m;
		
		std::queue<string> savior_save;
		std::mutex savior_save_m;
		//std::unordered_map<uintptr_t, std::unordered_map<int, int> > leaf_cache_i;
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

void forest::cache::leaf_unlock()
{
	//======//log_info_private("[cache::leaf_unlock] start");
	leaf_cache_m.unlock();
	
	//auto p1 = std::chrono::high_resolution_clock::now();
	//string key;
	//while(true){
	//	savior_save_m.lock();
	//	if(savior_save.empty()){
	//		savior_save_m.unlock();
	//		break;
	//	}
	//	key = savior_save.front();
	//	savior_save.pop();
	//	savior_save_m.unlock();
	//	
	//	savior->save(key,true);
	//}	
	//auto p2 = std::chrono::high_resolution_clock::now();
	//hook_remove_leaf += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
	//======//log_info_private("[cache::leaf_unlock] end");
}

void forest::cache::check_tree_ref(string& key)
{
	//======//log_info_private("[cache::check_tree_ref] start");
	if(!tree_cache_r.count(key)){
		//======//log_info_private("[cache::check_tree_ref] (no key found) end");
		return;
	}
	if(tree_cache_r[key].second == 0 && !tree_cache.has(key)){
		//savior->save(key, true);
		
		//std::cout << "SAVIOR_CHECK_TREE " + key + "\n";
		savior->lock_map();
		if(savior->has(key)){
			savior->unlock_map();			
			//std::cout << "SAVIOR_SAVE_TREE " + key + "\n";
			savior->save(key, true);
		} else {
			savior->unlock_map();
		}
		//std::cout << "REMOVE_TREE " + key + "\n";
		tree_cache_r.erase(key);
	}
	//======//log_info_private("[cache::check_tree_ref] end");
}

void forest::cache::check_leaf_ref(string& key)
{
	//======//log_info_private("[cache::check_leaf_ref] start");
	if(!leaf_cache_r.count(key)){
		//======//log_info_private("[cache::check_leaf_ref] (no key found) end");
		return;
	}
	auto& leaf_cache_ref = leaf_cache_r[key];
	if(leaf_cache_ref.second == 0 && !leaf_cache.has(key)){
		//====//std::cout << "LEAF_REF_START " + key + "\n";
		
		tree_t::node_ptr node = leaf_cache_ref.first;
		
		////get_data(node).f = nullptr;
		////for(auto& it : (*node->get_childs())){
			////it->item->second->set_file(nullptr);
			////it->item->second = nullptr;
			//it->node = nullptr;
		////}
		node->set_next_leaf(nullptr);
		node->set_prev_leaf(nullptr);
		
		// Close file if there if it should be closed
		auto p1 = std::chrono::high_resolution_clock::now();
		
		//std::cout << "+CACHE_MAP_FIRST_LOCK\n";
		savior->lock_map();
		//std::cout << "-CACHE_MAP_FIRST_LOCK\n";
		
		if(!savior->has(key) && get_data(node).f){
			 //====//std::cout << "CLOSE FILE WHICH IS NOT GOING TO BE SAVED " + key + "\n";
			get_data(node).f->close();
		}
		
		leaf_cache_r.erase(key);
		get_data(node).bloomed = false;
		
		if(savior->has(key)){
			//savior_save_m.lock();
			//savior_save.push(key);
			//savior_save_m.unlock();
			
			savior->unlock_map();
			savior->save(key, true);
		} else {
			savior->unlock_map();
		}
		
		auto p2 = std::chrono::high_resolution_clock::now();
		hook_remove_leaf += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
		//savior->save(key, true);

		
		//std::cout << "REMOVE_LEAF " + key + "\n";
		
		//====//std::cout << "LEAF_REF_END " + key + "\n";
	}
	//======//log_info_private("[cache::check_leaf_ref] end");
}

void forest::cache::check_intr_ref(string& key)
{
	//======//log_info_private("[cache::check_intr_ref] start");
	if(!intr_cache_r.count(key)){
		//======//log_info_private("[cache::check_intr_ref] () end");
		return;
	}
	auto& intr_cache_ref = intr_cache_r[key];
	if(intr_cache_ref.second == 0 && !intr_cache.has(key)){
		//intr_cache_r[key].first->get_nodes()->resize(0);
		//std::cout << "REMOVE_INTR " + key + "\n";
		
		savior->lock_map();
		if(savior->has(key)){
			savior->unlock_map();
			savior->save(key, true);
		} else {
			savior->unlock_map();
		}
		
		get_data(intr_cache_ref.first).bloomed = false;

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

void forest::cache::reserve_node(tree_t::node_ptr& node, bool w_lock)
{
	//======//log_info_private("[cache::reserve_node] start");
	assert(has_data(node));
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;
	
	if(is_leaf){
		if(w_lock){
			//with_lock(NODE_TYPES::LEAF, [&path](){
				leaf_lock();
				reserve_leaf_node(path);
				leaf_unlock();
			//});
		} else {
			reserve_leaf_node(path);
		}
	} else {
		if(w_lock){
			//with_lock(NODE_TYPES::INTR, [&path](){
				intr_lock();
				reserve_intr_node(path);
				intr_unlock();
			//});
		} else {
			reserve_intr_node(path);
		}
	}
	//======//log_info_private("[cache::reserve_node] end");
}

void forest::cache::release_node(tree_t::node_ptr& node, bool w_lock)
{
	//======//log_info_private("[cache::release_node] start");
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;

	if(is_leaf){
		if(w_lock){
			//with_lock(NODE_TYPES::LEAF, [&path](){
				leaf_lock();
				release_leaf_node(path);
				leaf_unlock();
			//});
		} else {
			release_leaf_node(path);
		}
	} else {
		if(w_lock){
			//with_lock(NODE_TYPES::INTR, [&path](){
				intr_lock();
				release_intr_node(path);
				intr_unlock();
			//});
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


void forest::cache::intr_insert(tree_t::node_ptr& node, bool w_lock)
{
	//======//log_info_private("[cache::intr_insert] start");
	if(w_lock){
		//with_lock(NODE_TYPES::INTR, [&node](){
			intr_lock();
			_intr_insert(node);
			intr_unlock();
		//});
	} else {
		_intr_insert(node);
	}
	//======//log_info_private("[cache::intr_insert] end");
}

void forest::cache::leaf_insert(tree_t::node_ptr& node, bool w_lock)
{
	//======//log_info_private("[cache::leaf_insert] start");
	if(w_lock){
		//with_lock(NODE_TYPES::LEAF, [&node](){
			leaf_lock();
			_leaf_insert(node);
			leaf_unlock();
		//});
	} else {
		_leaf_insert(node);
	}
	//======//log_info_private("[cache::leaf_insert] end");
}

void forest::cache::clear_node_cache(tree_t::node_ptr& node)
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
