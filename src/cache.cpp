#include "cache.hpp"

namespace forest{
	
	namespace cache{
		ListCache<string, tree_ptr> tree_cache(TREE_CACHE_LENGTH);
		ListCache<string, tree_t::node_ptr> leaf_cache(LEAF_CACHE_LENGTH), intr_cache(INTR_CACHE_LENGTH);
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		std::unordered_map<string, std::shared_future<tree_t::node_ptr> > intr_cache_q, leaf_cache_q;
		std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		std::unordered_map<string, std::pair<tree_t::node_ptr, int_a> > intr_cache_r, leaf_cache_r;
		std::unordered_map<string, std::unordered_map<int, int> > leaf_cache_i;
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

void forest::cache::check_tree_ref(string key)
{
	if(!tree_cache_r.count(key))
		return;
	if(tree_cache_r[key].second == 0 && !tree_cache.has(key)){
		tree_cache_r.erase(key);
	}
}

void forest::cache::check_leaf_ref(string key)
{
	if(!leaf_cache_r.count(key))
		return;
	if(leaf_cache_r[key].second == 0 && !leaf_cache.has(key)){
		tree_t::node_ptr node = leaf_cache_r[key].first;
		for(auto& it : (*node->get_childs())){
			it->item->second->set_file(nullptr);
			it->item->second = nullptr;
		}
		node->set_next_leaf(nullptr);
		node->set_prev_leaf(nullptr);
		leaf_cache_r.erase(key);
	}
}

void forest::cache::check_intr_ref(string key)
{
	if(!intr_cache_r.count(key))
		return;
	if(intr_cache_r[key].second == 0 && !intr_cache.has(key)){
		intr_cache_r[key].first->get_nodes()->resize(0);
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

void forest::cache::insert_item(string path, int pos)
{
	leaf_cache_i[path][pos]++;
}

void forest::cache::remove_item(string path, int pos)
{
	auto& c_node = leaf_cache_i[path];
	
	assert(c_node[pos] > 0);
	
	c_node[pos]--;
	if(!c_node[pos]){
		c_node.erase(pos);
	}
	if(!c_node.size()){
		leaf_cache_i.erase(path);
	}
}

void forest::cache::intr_lock()
{
	intr_cache_m.lock();
}

void forest::cache::intr_unlock()
{
	intr_cache_m.unlock();
}

void forest::cache::leaf_lock()
{
	leaf_cache_m.lock();
}

void forest::cache::leaf_unlock()
{
	leaf_cache_m.unlock();
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
}

void forest::cache::release_node(tree_t::node_ptr node, bool w_lock)
{
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
	intr_cache_r[path].second++;
}

void forest::cache::release_intr_node(string& path)
{
	assert(intr_cache_r.count(path));
	assert(intr_cache_r[path].second>0);
	intr_cache_r[path].second--;
	check_intr_ref(path);
}

void forest::cache::reserve_leaf_node(string& path)
{
	leaf_cache_r[path].second++;
}

void forest::cache::release_leaf_node(string& path)
{
	assert(leaf_cache_r.count(path));
	assert(leaf_cache_r[path].second>0);
	leaf_cache_r[path].second--;
	check_leaf_ref(path);
}

void forest::cache::intr_insert(tree_t::node_ptr node, bool w_lock)
{
	if(w_lock){
		with_lock(NODE_TYPES::INTR, [&node](){
			_intr_insert(node);
		});
	} else {
		_intr_insert(node);
	}
}

void forest::cache::leaf_insert(tree_t::node_ptr node, bool w_lock)
{
	if(w_lock){
		with_lock(NODE_TYPES::LEAF, [&node](){
			_leaf_insert(node);
		});
	} else {
		_leaf_insert(node);
	}
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

