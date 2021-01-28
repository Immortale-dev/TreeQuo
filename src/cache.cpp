#include "cache.hpp"

extern int hook_remove_leaf;

namespace forest{
namespace details{
	
	namespace cache{
		ListCache<string, tree_ptr> tree_cache(TREE_CACHE_LENGTH);
		//ListCache<string, tree_t::node_ptr> leaf_cache(LEAF_CACHE_LENGTH), intr_cache(INTR_CACHE_LENGTH);
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, tree_cache_ref_t*> tree_cache_r;
		std::unordered_map<string, node_cache_ref_t*> leaf_cache_r;
		std::unordered_map<string, node_cache_ref_t*> intr_cache_r;
		
		std::list<node_ptr> leaf_cache_l, intr_cache_l;
	}
	
} // details
} // forest

/////////////CACHE_METHODS//////////////

void forest::details::cache::init_cache()
{
	tree_cache.set_callback([](string key){ forest::details::cache::check_tree_ref(key); });
	//leaf_cache.set_callback([](string key){ forest::details::cache::check_leaf_ref(key); });
	//intr_cache.set_callback([](string key){ forest::details::cache::check_intr_ref(key); });
}


void forest::details::cache::leaf_cache_push(node_ptr node)
{
	auto& node_data = get_data(node);
	if(!node_data.cache_iterator_valid){
		leaf_cache_l.push_front(node);
		node_data.cache_iterator = leaf_cache_l.begin();
		node_data.cache_iterator_valid = true;
		if(leaf_cache_l.size() > (size_t)LEAF_CACHE_LENGTH){
			node = leaf_cache_l.back();
			get_data(node).cache_iterator_valid = false;
			leaf_cache_l.pop_back();
			check_leaf_ref(node);
		}
	} else {
		leaf_cache_l.splice(leaf_cache_l.begin(), leaf_cache_l, node_data.cache_iterator);
	}
}

void forest::details::cache::intr_cache_push(node_ptr node)
{
	auto& node_data = get_data(node);
	if(!node_data.cache_iterator_valid){
		intr_cache_l.push_front(node);
		node_data.cache_iterator = intr_cache_l.begin();
		node_data.cache_iterator_valid = true;
		if(intr_cache_l.size() > (size_t)INTR_CACHE_LENGTH){
			node = intr_cache_l.back();
			get_data(node).cache_iterator_valid = false;
			intr_cache_l.pop_back();
			check_intr_ref(node);
		}
	} else {
		intr_cache_l.splice(intr_cache_l.begin(), intr_cache_l, node_data.cache_iterator);
	}
}

void forest::details::cache::leaf_cache_remove(node_ptr node)
{
	auto& node_data = get_data(node);
	if(!node_data.cache_iterator_valid)
		return;
	leaf_cache_l.erase(node_data.cache_iterator);
	node_data.cache_iterator_valid = false;
	check_leaf_ref(node);
}

void forest::details::cache::intr_cache_remove(node_ptr node)
{
	auto& node_data = get_data(node);
	if(!node_data.cache_iterator_valid)
		return;
	intr_cache_l.erase(node_data.cache_iterator);
	node_data.cache_iterator_valid = false;
	check_intr_ref(node);
}

void forest::details::cache::leaf_cache_clear()
{
	while(!leaf_cache_l.empty()){
		node_ptr node = leaf_cache_l.front();
		get_data(node).cache_iterator_valid = false;
		leaf_cache_l.pop_front();
		check_leaf_ref(node);
	}
}

void forest::details::cache::intr_cache_clear()
{
	while(!intr_cache_l.empty()){
		node_ptr node = intr_cache_l.front();
		get_data(node).cache_iterator_valid = false;
		intr_cache_l.pop_front();
		check_intr_ref(node);
	}
}

void forest::details::cache::release_cache()
{
	//leaf_cache.clear();
	//intr_cache.clear();
	leaf_cache_clear();
	intr_cache_clear();
	tree_cache.clear();
}

void forest::details::cache::leaf_unlock()
{
	leaf_cache_m.unlock();
}

void forest::details::cache::check_tree_ref(string key)
{
	if(!tree_cache_r.count(key)){
		return;
	}
	auto* tree_cache_ref = tree_cache_r[key];
	
	ASSERT(tree_cache_ref->second >= 0);
	
	if(tree_cache_ref->second == 0 && !tree_cache.has(key)){
		tree_ptr tree = tree_cache_ref->first;
		tree_cache_r.erase(key);
		delete tree_cache_ref;
		savior->leave(key, SAVE_TYPES::BASE, tree);
	}
}

void forest::details::cache::check_leaf_ref(node_ptr node)
{
	auto* leaf_cache_ref = get_data(node).cached_ref;
	if(!leaf_cache_ref) return;
	
	ASSERT(leaf_cache_ref->second >= 0);
	
	if(leaf_cache_ref->second == 0 && !get_data(node).cache_iterator_valid){
		node->set_next_leaf(nullptr);
		node->set_prev_leaf(nullptr);
		
		delete leaf_cache_ref;
		
		string& key = get_node_data(node)->path;
		
		leaf_cache_r.erase(key);
		get_data(node).bloomed = false;
		get_data(node).cached_ref = nullptr;
		
		savior->leave(key, SAVE_TYPES::LEAF, node);
	}
}

void forest::details::cache::check_intr_ref(node_ptr node)
{
	auto* intr_cache_ref = get_data(node).cached_ref;
	
	ASSERT(intr_cache_ref->second >= 0);
	
	if(intr_cache_ref->second == 0 && !get_data(node).cache_iterator_valid){
		delete intr_cache_ref;
		
		string& key = get_node_data(node)->path;
		
		intr_cache_r.erase(key);
		get_data(node).bloomed = false;
		get_data(node).cached_ref = nullptr;
		
		savior->leave(key, SAVE_TYPES::INTR, node);
	}
}

void forest::details::cache::set_tree_cache_length(int length)
{
	TREE_CACHE_LENGTH = length;
	tree_cache_m.lock();
	tree_cache.resize(TREE_CACHE_LENGTH);
	tree_cache_m.unlock();
}

void forest::details::cache::set_intr_cache_length(int length)
{
	INTR_CACHE_LENGTH = length;
	intr_cache_m.lock();
	///intr_cache.resize(INTR_CACHE_LENGTH);
	intr_cache_m.unlock();
}

void forest::details::cache::set_leaf_cache_length(int length)
{
	LEAF_CACHE_LENGTH = length;
	leaf_cache_m.lock();
	///leaf_cache.resize(LEAF_CACHE_LENGTH);
	leaf_cache_m.unlock();
}

void forest::details::cache::reserve_node(tree_t::node_ptr& node, bool w_lock)
{
	ASSERT(has_data(node));
	
	bool is_leaf = node->is_leaf();
	//string& path = get_node_data(node)->path;
	
	if(is_leaf){
		if(w_lock){
			leaf_lock();
			reserve_leaf_node(node);
			leaf_unlock();
		} else {
			reserve_leaf_node(node);
		}
	} else {
		if(w_lock){
			intr_lock();
			reserve_intr_node(node);
			intr_unlock();
		} else {
			reserve_intr_node(node);
		}
	}
}

void forest::details::cache::release_node(tree_t::node_ptr& node, bool w_lock)
{
	bool is_leaf = node->is_leaf();
	//string& path = get_node_data(node)->path;

	if(is_leaf){
		if(w_lock){
			leaf_lock();
			release_leaf_node(node);
			leaf_unlock();
		} else {
			release_leaf_node(node);
		}
	} else {
		if(w_lock){
			intr_lock();
			release_intr_node(node);
			intr_unlock();
		} else {
			release_intr_node(node);
		}
	}
}

void forest::details::cache::with_lock(NODE_TYPES type, std::function<void()> fn)
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


void forest::details::cache::intr_insert(tree_t::node_ptr& node, bool w_lock)
{
	if(w_lock){
		intr_lock();
		_intr_insert(node);
		intr_unlock();
	} else {
		_intr_insert(node);
	}
}

void forest::details::cache::leaf_insert(tree_t::node_ptr& node, bool w_lock)
{
	if(w_lock){
		leaf_lock();
		_leaf_insert(node);
		leaf_unlock();
	} else {
		_leaf_insert(node);
	}
}

void forest::details::cache::clear_node_cache(tree_t::node_ptr& node)
{
	node_data_ptr data = get_node_data(node);
	//string& path = data->path;
	if(node->is_leaf()){
		cache::leaf_lock();
		leaf_cache_remove(node);
		//if(cache::leaf_cache.has(path)){
		//	cache::leaf_cache.remove(path);
		//}
		cache::leaf_unlock();
	}
	else{
		cache::intr_lock();
		intr_cache_remove(node);
		//if(cache::intr_cache.has(path)){
		//	cache::intr_cache.remove(path);
		//}
		cache::intr_unlock();
	}
}
