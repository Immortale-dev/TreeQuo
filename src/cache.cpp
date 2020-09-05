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
	}
	
	tree_ptr FOREST;
	bool blossomed = false;
}

/////////////CACHE_METHODS//////////////

void forest::cache::init_cache()
{
	tree_cache.set_callback([](string key){ forest::cache::check_tree_ref(key); });
	leaf_cache.set_callback([](string key){ forest::cache::check_leaf_ref(key); });
	intr_cache.set_callback([](string key){ forest::cache::check_intr_ref(key); });
}

void forest::cache::release_cache()
{
	// Remove?
}

void forest::cache::leaf_unlock()
{
	leaf_cache_m.unlock();
}

void forest::cache::check_tree_ref(string key)
{
	if(!tree_cache_r.count(key)){
		return;
	}
	if(tree_cache_r[key].second == 0 && !tree_cache.has(key)){
		tree_cache_r.erase(key);
		savior->lock_map();
		if(savior->has(key)){
			savior->unlock_map();			
			savior->save(key, false);
		} else {
			savior->unlock_map();
		}
	}
}

void forest::cache::check_leaf_ref(string& key)
{
	if(!leaf_cache_r.count(key)){
		return;
	}
	auto& leaf_cache_ref = leaf_cache_r[key];
	if(leaf_cache_ref.second == 0 && !leaf_cache.has(key)){
		tree_t::node_ptr node = leaf_cache_ref.first;
		
		node->set_next_leaf(nullptr);
		node->set_prev_leaf(nullptr);
		
		// Close file if there if it should be closed
		savior->lock_map();
		if(!savior->has(key) && get_data(node).f){
			get_data(node).f->close();
		}
		
		leaf_cache_r.erase(key);
		
		own_lock(node);
		get_data(node).bloomed = false;
		own_unlock(node);
		
		if(savior->has(key)){
			savior->save(key, false);
		}
		savior->unlock_map();
	}
}

void forest::cache::check_intr_ref(string& key)
{
	if(!intr_cache_r.count(key)){
		return;
	}
	auto& intr_cache_ref = intr_cache_r[key];
	if(intr_cache_ref.second == 0 && !intr_cache.has(key)){
		savior->lock_map();
		if(savior->has(key)){
			savior->unlock_map();
			savior->save(key, false);
		} else {
			savior->unlock_map();
		}
		get_data(intr_cache_ref.first).bloomed = false;
		intr_cache_r.erase(key);
	}
}

void forest::cache::set_tree_cache_length(int length)
{
	TREE_CACHE_LENGTH = length;
	tree_cache_m.lock();
	tree_cache.resize(TREE_CACHE_LENGTH);
	tree_cache_m.unlock();
}

void forest::cache::set_intr_cache_length(int length)
{
	INTR_CACHE_LENGTH = length;
	intr_cache_m.lock();
	intr_cache.resize(INTR_CACHE_LENGTH);
	intr_cache_m.unlock();
}

void forest::cache::set_leaf_cache_length(int length)
{
	LEAF_CACHE_LENGTH = length;
	leaf_cache_m.lock();
	leaf_cache.resize(LEAF_CACHE_LENGTH);
	leaf_cache_m.unlock();
}

void forest::cache::reserve_node(tree_t::node_ptr& node, bool w_lock)
{
	assert(has_data(node));
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;
	
	if(is_leaf){
		if(w_lock){
			leaf_lock();
			reserve_leaf_node(path);
			leaf_unlock();
		} else {
			reserve_leaf_node(path);
		}
	} else {
		if(w_lock){
			intr_lock();
			reserve_intr_node(path);
			intr_unlock();
		} else {
			reserve_intr_node(path);
		}
	}
}

void forest::cache::release_node(tree_t::node_ptr& node, bool w_lock)
{
	bool is_leaf = node->is_leaf();
	string& path = get_node_data(node)->path;

	if(is_leaf){
		if(w_lock){
			leaf_lock();
			release_leaf_node(path);
			leaf_unlock();
		} else {
			release_leaf_node(path);
		}
	} else {
		if(w_lock){
			intr_lock();
			release_intr_node(path);
			intr_unlock();
		} else {
			release_intr_node(path);
		}
	}
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
	if(w_lock){
		intr_lock();
		_intr_insert(node);
		intr_unlock();
	} else {
		_intr_insert(node);
	}
}

void forest::cache::leaf_insert(tree_t::node_ptr& node, bool w_lock)
{
	if(w_lock){
		leaf_lock();
		_leaf_insert(node);
		leaf_unlock();
	} else {
		_leaf_insert(node);
	}
}

void forest::cache::clear_node_cache(tree_t::node_ptr& node)
{
	node_data_ptr data = get_node_data(node);
	string& path = data->path;
	if(node->is_leaf()){
		cache::leaf_lock();
		if(cache::leaf_cache.has(path)){
			cache::leaf_cache.remove(path);
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
}
