#include "forest.hpp"

namespace forest{
	Savior* savior;
	bool folding = false;
}


void forest::bloom(string path)
{	
	L_PUB("[forest::bloom]-start");
	
	if(blossomed){
		return;
	}

	cache::init_cache();

	DBFS::root = path;
	if(!DBFS::exists(ROOT_TREE)){
		details::create_root_file();
	} 

	savior = new Savior();
	
	details::open_root();

	blossomed = true;
	
	L_PUB("[forest::bloom]-end");
}

void forest::fold()
{	
	L_PUB("[forest::fold]-start");
	
	if(!blossomed){
		return;
	}
		
	folding = true;
	blossomed = false;
	
	cache::release_cache();
	
	delete savior;
	
	details::close_root();
	
	Thread_wait::wait();
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	L_PUB("[forest::fold]-end");
}

void forest::create_tree(TREE_TYPES type, string name, int factor, string annotation)
{
	L_PUB("[forest::create_tree]-" + name);
	
	if(!factor){
		factor = DEFAULT_FACTOR;
	}
	
	{
		if(FOREST->get_tree()->find(name) != FOREST->get_tree()->end()){
			throw DBException(DBException::ERRORS::TREE_ALREADY_EXISTS);
		}
	}
	
	string file_name = DBFS::random_filename();
	
	tree_ptr tree = tree_ptr(new Tree(file_name, type, factor, annotation));
	
	details::insert_tree(name, file_name, tree);
}

void forest::delete_tree(string name)
{
	L_PUB("[forest::delete_tree]-" + name);
	
	string path;
	// Not exist
	{
		auto it = FOREST->get_tree()->find(name);
		if(it == FOREST->get_tree()->end()){
			return;
		}
		
		// Get tree path
		path = read_leaf_item(it->second);
	}
	
	// Remove tree from forest
	FOREST->erase(name);
	
	// Erase tree
	details::erase_tree(path);
}

void forest::leave_tree(string path)
{	
	L_PUB("[forest::leave_tree]-" + path);
	cache::tree_cache_m.lock();
	assert(cache::tree_cache_r.count(path));
	assert(cache::tree_cache_r[path].second > 0);
	cache::tree_cache_r[path].second--;
	cache::check_tree_ref(path);
	cache::tree_cache_m.unlock();
}

void forest::leave_tree(tree_ptr tree)
{
	leave_tree(tree->get_name());
}

forest::tree_ptr forest::find_tree(string name)
{	
	L_PUB("[forest::find_tree]-" + name);
	
	// Error if not exists
	auto it = FOREST->get_tree()->find(name);
	if(it == FOREST->get_tree()->end()){
		throw DBException(DBException::ERRORS::TREE_DOES_NOT_EXISTS);
	}
	
	// Get tree path
	string path = read_leaf_item(it->second);
	return open_tree(path);
}

forest::tree_ptr forest::open_tree(string path)
{	
	L_PUB("[forest::open_tree]-" + path);
	
	cache::tree_cache_m.lock();
	tree_ptr t = details::get_tree(path);
	cache::tree_cache_r[path].second++;
	cache::tree_cache_m.unlock();
	t->ready();
	return t;
}

void forest::insert_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{	
	tree_ptr tree = find_tree(name);
	
	L_PUB("[forest::insert_leaf]-" + tree->get_name() + "_" + key);
	
	tree->insert(key, std::move(val));
	leave_tree(tree->get_name());
}

void forest::insert_leaf(tree_ptr tree, tree_t::key_type key, tree_t::val_type val)
{
	L_PUB("[forest::insert_leaf]-" + tree->get_name() + "_" + key);
	
	tree->insert(key, std::move(val));
}

void forest::update_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{	
	tree_ptr tree = find_tree(name);
	
	L_PUB("[forest::update_leaf]-" + tree->get_name() + "_" + key);
	
	tree->insert(key, std::move(val), true);
	leave_tree(tree->get_name());
}

void forest::update_leaf(tree_ptr tree, tree_t::key_type key, tree_t::val_type val)
{	
	L_PUB("[forest::update_leaf]-" + tree->get_name() + "_" + key);
	
	tree->insert(key, std::move(val), true);
}

void forest::erase_leaf(string name, tree_t::key_type key)
{	
	tree_ptr tree = find_tree(name);
	
	L_PUB("[forest::erase_leaf]-" + tree->get_name() + "_" + key);
	
	tree->erase(key);
	leave_tree(tree->get_name());
}

void forest::erase_leaf(tree_ptr tree, tree_t::key_type key)
{	
	L_PUB("[forest::erase_leaf]-" + tree->get_name() + "_" + key);
	
	tree->erase(key);
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key)
{	
	tree_ptr tree = find_tree(name);
	try{
		L_PUB("[forest::find_leaf]-KEY_" + key);
		
		tree_t::iterator t = tree->find(key);
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		leave_tree(tree->get_name());
		return rc;
	} 
	catch(DBException& e) {
		leave_tree(tree->get_name());
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(string name, RECORD_POSITION position)
{	
	tree_ptr tree = find_tree(name);
	try{
		L_PUB("[forest::find_leaf]-POS_" + to_string((int)position));
		
		tree_t::iterator t;
		if(position == RECORD_POSITION::BEGIN){
			t = tree->get_tree()->begin();
		}
		else{
			t = --tree->get_tree()->end();
		}
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		leave_tree(tree->get_name());
		
		return rc;
	}
	catch(DBException& e) {
		leave_tree(tree->get_name());
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key, RECORD_POSITION position)
{	
	if(position == RECORD_POSITION::BEGIN || position == RECORD_POSITION::END){
		return find_leaf(name, position);
	}
	
	tree_ptr tree = find_tree(name);
	try{
		L_PUB("[forest::find_leaf]-BNT_" + key + "_" + to_string((int)position));
		
		tree_t::iterator t;
		if(position == RECORD_POSITION::LOWER){
			t = tree->get_tree()->lower_bound(key);
		}
		else {
			t = tree->get_tree()->upper_bound(key);
		}
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		leave_tree(tree->get_name());
		
		return rc;
	}
	catch(DBException& e) {
		leave_tree(tree->get_name());
		throw e;
	}
}


forest::LeafRecord_ptr forest::find_leaf(tree_ptr tree, tree_t::key_type key)
{	
	try{
		L_PUB("[forest::find_leaf]-KEY_" + key);
		tree_t::iterator t = tree->find(key);
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		return rc;
	} catch(DBException& e) {
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(tree_ptr tree, RECORD_POSITION position)
{	
	try{
		L_PUB("[forest::find_leaf]-POS_" + to_string((int)position));
		
		tree_t::iterator t;
		if(position == RECORD_POSITION::BEGIN){
			t = tree->get_tree()->begin();
		} else {
			t = --tree->get_tree()->end();
		}
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		return rc;
	} catch(DBException& e) {
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(tree_ptr tree, tree_t::key_type key, RECORD_POSITION position)
{	
	if(position == RECORD_POSITION::BEGIN || position == RECORD_POSITION::END){
		return find_leaf(tree, position);
	}
	
	try{
		L_PUB("[forest::find_leaf]-BNT_" + key + "_" + to_string((int)position));
		
		tree_t::iterator t;
		if(position == RECORD_POSITION::LOWER){
			t = tree->get_tree()->lower_bound(key);
		} else {
			t = tree->get_tree()->upper_bound(key);
		}
		
		LeafRecord_ptr rc = LeafRecord_ptr(new LeafRecord(t, tree));
		
		return rc;
	} catch(DBException& e) {
		throw e;
	}
}


/* Configurations */

void forest::config_root_factor(int root_factor)
{
	ROOT_FACTOR = root_factor;
}

void forest::config_default_factor(int default_factor)
{
	DEFAULT_FACTOR = default_factor;
}

void forest::config_root_tree(string root_tree)
{
	ROOT_TREE = root_tree;
}

void forest::config_intr_cache_length(int length)
{
	cache::set_intr_cache_length(length);
}

void forest::config_leaf_cache_length(int length)
{
	cache::set_leaf_cache_length(length);
}

void forest::config_tree_cache_length(int length)
{
	cache::set_tree_cache_length(length);
}

void forest::config_cache_bytes(int bytes)
{
	CACHE_BYTES = bytes;
}

void forest::config_chunk_bytes(int bytes)
{
	CHUNK_SIZE = bytes;
}

void forest::config_opened_files_limit(int count)
{
	OPENED_FILES_LIMIT = count;
}

void forest::config_save_schedule_mks(int mks)
{
	SCHEDULE_TIMER = mks;
}

/*********************************************************************************/


void forest::details::create_root_file()
{
	Tree::seed(TREE_TYPES::KEY_STRING, ROOT_TREE, ROOT_FACTOR);
}

void forest::details::open_root()
{
	FOREST = tree_ptr(new Tree(ROOT_TREE));
}

void forest::details::close_root()
{
	FOREST = nullptr;
}

void forest::details::insert_tree(string name, string file_name, tree_ptr tree)
{
	cache::tree_cache_m.lock();
	cache::tree_cache.push(file_name, tree);
	cache::tree_cache_r[file_name] = make_pair(tree,1);
	cache::tree_cache_m.unlock();
	
	savior->put(file_name, SAVE_TYPES::BASE, tree);
	
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, std::move(tmp));
	
	cache::tree_cache_m.lock();
	assert(cache::tree_cache_r.count(file_name));
	assert(cache::tree_cache_r[file_name].second > 0);
	cache::tree_cache_r[file_name].second--;
	cache::check_tree_ref(file_name);
	cache::tree_cache_m.unlock();
}

void forest::details::erase_tree(string path)
{	
	tree_ptr t = open_tree(path);
	
	t->get_tree()->lock_write();
	savior->remove(path, SAVE_TYPES::BASE, t);
	t->get_tree()->unlock_write();
	
	t->get_tree()->clear();

	// Clear cache
	cache::tree_cache_m.lock();
	if(cache::tree_cache.has(path)){
		cache::tree_cache.remove(path);
	}
	cache::tree_cache_m.unlock();
	
	leave_tree(path);
}

forest::tree_ptr forest::details::get_tree(string path)
{
	return Tree::get(path);
}

forest::string forest::read_leaf_item(file_data_ptr item)
{	
	int sz = item->size();
	char* buf = new char[sz];
	auto reader = item->get_reader();
	reader.read(buf,sz);
	string ret(buf,sz);
	delete[] buf;
	
	return ret;
}
