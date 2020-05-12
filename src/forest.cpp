#include "forest.hpp"

void forest::bloom(string path)
{
	if(blossomed)
		return;

	cache::init_cache();

	DBFS::root = path;
	if(!DBFS::exists(ROOT_TREE)){
		create_root_file();
	}

	open_root();

	blossomed = true;
}

void forest::fold()
{
	if(!blossomed)
		return;
		
	blossomed = false;
	
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
	string file_name = Tree::seed(type, factor);
	insert_tree(name, file_name);
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

void forest::close_tree(string path)
{
	cache::tree_cache_m.lock();
	cache::tree_cache_r[path].second--;
	cache::check_tree_ref(path);
	cache::tree_cache_m.unlock();
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
	cache::tree_cache_m.unlock();;
	return t;
}

void forest::insert_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{
	tree_ptr tree = find_tree(name);
	tree->insert(key, val);
	close_tree(tree->get_name());
}

void forest::update_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{
	tree_ptr tree = find_tree(name);
	tree->insert(key, val, true);
	close_tree(tree->get_name());
}

void forest::erase_leaf(string name, tree_t::key_type key)
{
	tree_ptr tree = find_tree(name);
	tree->erase(key);
	close_tree(tree->get_name());
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key)
{
	tree_ptr tree = find_tree(name);
	try{
		tree_t::iterator t = tree->find(key);
		close_tree(tree->get_name());
		return LeafRecord_ptr(new LeafRecord(t));
	} 
	catch(DBException& e) {
		close_tree(tree->get_name());
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
			t = tree->get_tree()->end();
			--t;
		}
		close_tree(tree->get_name());
		return LeafRecord_ptr(new LeafRecord(t));
	}
	catch(DBException& e) {
		close_tree(tree->get_name());
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
	// Remove?
}

void forest::insert_tree(string name, string file_name)
{
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, tmp);
}

void forest::erase_tree(string path)
{
	tree_ptr t = open_tree(path);
	
	t->get_tree()->clear();
	
	DBFS::remove(path);

	// Clear cache
	cache::tree_cache_m.lock();
	if(cache::tree_cache_r.count(path)){
		cache::tree_cache_r.erase(path);
	}
	if(cache::tree_cache.has(path)){
		cache::tree_cache.remove(path);
	}
	cache::tree_cache_m.unlock();
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

