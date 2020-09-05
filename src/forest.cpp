#include "forest.hpp"

namespace forest{
	Savior* savior;
}

void forest::bloom(string path)
{	
	if(blossomed){
		return;
	}

	cache::init_cache();

	DBFS::root = path;
	if(!DBFS::exists(ROOT_TREE)){
		create_root_file();
	}

	savior = new Savior();
	
	open_root();

	blossomed = true;
}

void forest::fold()
{	
	if(!blossomed){
		return;
	}
		
	blossomed = false;
	
	delete savior;
	
	
	close_root();
	
	cache::release_cache();
}

void forest::create_tree(TREE_TYPES type, string name, int factor)
{
	if(!factor){
		factor = DEFAULT_FACTOR;
	}
	
	{
		if(FOREST->get_tree()->find(name) != FOREST->get_tree()->end()){
			throw DBException(DBException::ERRORS::TREE_ALREADY_EXISTS);
		}
	}
	
	string file_name = DBFS::random_filename();
	
	tree_ptr tree = tree_ptr(new Tree(file_name, type, factor));
	
	insert_tree(name, file_name, tree);
}

void forest::delete_tree(string name)
{
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
	erase_tree(path);
}

void forest::leave_tree(string path)
{
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
	cache::tree_cache_m.lock();
	tree_ptr t = get_tree(path);
	cache::tree_cache_r[path].second++;
	cache::tree_cache_m.unlock();
	return t;
}

void forest::insert_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{	
	tree_ptr tree = find_tree(name);
	tree->insert(key, std::move(val));
	leave_tree(tree->get_name());
}

void forest::update_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{	
	tree_ptr tree = find_tree(name);
	tree->insert(key, std::move(val), true);
	leave_tree(tree->get_name());
}

void forest::erase_leaf(string name, tree_t::key_type key)
{	
	tree_ptr tree = find_tree(name);
	tree->erase(key);
	leave_tree(tree->get_name());
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key)
{	
	tree_ptr tree = find_tree(name);
	try{
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


void forest::create_root_file()
{
	Tree::seed(TREE_TYPES::KEY_STRING, ROOT_TREE, ROOT_FACTOR);
}

void forest::open_root()
{
	FOREST = tree_ptr(new Tree(ROOT_TREE));
}

void forest::close_root()
{
	FOREST = nullptr;
}

void forest::insert_tree(string name, string file_name, tree_ptr tree)
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

void forest::insert_tree(string name, string file_name)
{
	//REMOVE???
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, tmp);
}

void forest::erase_tree(string path)
{	
	tree_ptr t = open_tree(path);
	
	t->get_tree()->clear();
	
	savior->remove(path, SAVE_TYPES::BASE, t);

	// Clear cache
	cache::tree_cache_m.lock();
	if(cache::tree_cache.has(path)){
		cache::tree_cache.remove(path);
	}
	cache::tree_cache_m.unlock();
	
	leave_tree(path);
}

forest::tree_ptr forest::get_tree(string path)
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

