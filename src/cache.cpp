#include "cache.hpp"

extern int hook_remove_leaf;

namespace forest{
namespace details{
	
	namespace cache{
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, tree_cache_ref_t*> tree_cache_r;
		std::unordered_map<string, node_cache_ref_t*> leaf_cache_r;
		std::unordered_map<string, node_cache_ref_t*> intr_cache_r;
		
		std::list<node_ptr> leaf_cache_l, intr_cache_l;
		std::list<tree_ptr> tree_cache_l;
	}
	
} // details
} // forest

/////////////CACHE_METHODS//////////////

void forest::details::cache::init_cache()
{
	// No need to initiate
}


void forest::details::cache::leaf_cache_push(node_ptr node)
{
	auto& node_data = get_data(node);
	if(!node_data.cache_iterator_valid){
		leaf_cache_l.push_front(node);
		node_data.cache_iterator = leaf_cache_l.begin();
		node_data.cache_iterator_valid = true;
		while(leaf_cache_l.size() > (size_t)LEAF_CACHE_LENGTH){
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
		while(intr_cache_l.size() > (size_t)INTR_CACHE_LENGTH){
			node = intr_cache_l.back();
			get_data(node).cache_iterator_valid = false;
			intr_cache_l.pop_back();
			check_intr_ref(node);
		}
	} else {
		intr_cache_l.splice(intr_cache_l.begin(), intr_cache_l, node_data.cache_iterator);
	}
}

void forest::details::cache::tree_cache_push(tree_ptr tree)
{
	auto& cached = tree->get_cached();
	if(!cached.iterator_valid){
		tree_cache_l.push_front(tree);
		cached.iterator = tree_cache_l.begin();
		cached.iterator_valid = true;
		while(tree_cache_l.size() > (size_t)TREE_CACHE_LENGTH){
			tree = tree_cache_l.back();
			tree->get_cached().iterator_valid = false;
			tree_cache_l.pop_back();
			check_tree_ref(tree);
		}
	} else {
		tree_cache_l.splice(tree_cache_l.begin(), tree_cache_l, cached.iterator);
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

void forest::details::cache::tree_cache_remove(tree_ptr tree)
{
	auto& cached = tree->get_cached();
	if(!cached.iterator_valid)
		return;
	tree_cache_l.erase(cached.iterator);
	cached.iterator_valid = false;
	check_tree_ref(tree);
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

void forest::details::cache::tree_cache_clear()
{
	while(!tree_cache_l.empty()){
		tree_ptr tree = tree_cache_l.front();
		tree->get_cached().iterator_valid = false;
		tree_cache_l.pop_front();
		check_tree_ref(tree);
	}
}

void forest::details::cache::release_cache()
{
	leaf_cache_clear();
	intr_cache_clear();
	tree_cache_clear();
}

void forest::details::cache::leaf_unlock()
{
	leaf_cache_m.unlock();
}

void forest::details::cache::check_tree_ref(tree_ptr tree)
{
	auto* tree_cache_ref = tree->get_cached().ref;
	if(!tree_cache_ref) return;
	
	ASSERT(tree_cache_ref->second >= 0);
	
	if(tree_cache_ref->second == 0 && !tree->get_cached().iterator_valid){
		tree_ptr tree = tree_cache_ref->first;
		delete tree_cache_ref;
		string key = tree->get_name();
		tree_cache_r.erase(key);
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
}

void forest::details::cache::set_intr_cache_length(int length)
{
	INTR_CACHE_LENGTH = length;
}

void forest::details::cache::set_leaf_cache_length(int length)
{
	LEAF_CACHE_LENGTH = length;
}

void forest::details::cache::reserve_node(tree_t::node_ptr& node, bool w_lock)
{
	ASSERT(has_data(node));
	
	bool is_leaf = node->is_leaf();
	
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

void forest::details::cache::reserve_tree(tree_ptr tree)
{
	ASSERT(tree->get_cached().ref->second >= 0);
	++tree->get_cached().ref->second;
}

void forest::details::cache::release_tree(tree_ptr tree)
{
	ASSERT(tree->get_cached().ref->second > 0);
	if(--tree->get_cached().ref->second == 0){
		cache::check_tree_ref(tree);
	}
}

void forest::details::cache::release_node(tree_t::node_ptr& node, bool w_lock)
{
	bool is_leaf = node->is_leaf();

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
	if(node->is_leaf()){
		cache::leaf_lock();
		leaf_cache_remove(node);
		cache::leaf_unlock();
	}
	else{
		cache::intr_lock();
		intr_cache_remove(node);
		cache::intr_unlock();
	}
}
