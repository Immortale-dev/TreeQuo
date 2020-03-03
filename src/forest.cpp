#include "forest.hpp"

namespace forest{
	const string ROOT_TREE = "_root";
	int DEFAULT_FACTOR = 3;
	int INTR_CACHE_LENGTH = 10;
	int LEAF_CACHE_LENGTH = 10;
	int TREE_CACHE_LENGTH = 10;
	
	namespace cache{
		ListCache<string, tree_ptr> tree_cache(TREE_CACHE_LENGTH);
		ListCache<string, tree_t::node_ptr> leaf_cache(LEAF_CACHE_LENGTH), intr_cache(INTR_CACHE_LENGTH);
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::shared_future<tree_ptr> > tree_cache_q;
		std::unordered_map<string, std::shared_future<tree_t::node_ptr> > intr_cache_q, leaf_cache_q;
		std::unordered_map<string, std::pair<tree_ptr, int_a> > tree_cache_r;
		std::unordered_map<string, std::pair<tree_t::node_ptr, int_a> > intr_cache_r, leaf_cache_r;
	}
	
	tree_ptr FOREST;
	bool blossomed = false;
}

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



void forest::create_tree(TREE_TYPES type, string name)
{
	if(FOREST->get_tree()->find(name) != FOREST->get_tree()->end()){
		throw DBException(DBException::ERRORS::TREE_ALREADY_EXISTS);
	}
	string file_name = Tree::seed(type);
	insert_tree(name, file_name);
}

void forest::delete_tree(string name)
{
	// Not exist
	auto it = FOREST->get_tree()->find(name);
	if(it == FOREST->get_tree()->end()){
		return;
	}
	
	// Get tree path
	string path = read_leaf_item(it->second);
	
	// Remove tree from forest
	FOREST->erase(name);
	
	// Erase tree
	erase_tree(path);
}

void forest::close_tree(string path)
{
	cache::tree_cache_r[path].second--;
	cache::tree_cache_m.lock();
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
	tree_ptr t = get_tree(path);
	cache::tree_cache_r[path].second++;
	return t;
}

void forest::insert_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{
	tree_ptr tree = find_tree(name);
	tree->insert(key, val);
	close_tree(tree->get_name());
}

void forest::erase_leaf(string name, tree_t::key_type key)
{
	tree_ptr tree = find_tree(name);
	tree->erase(key);
	close_tree(tree->get_name());
}

forest::file_data_ptr forest::find_leaf(string name, tree_t::key_type key)
{
	tree_ptr tree = find_tree(name);
	try{
		file_data_ptr t = tree->find(key);
		close_tree(tree->get_name());
		return t;
	} 
	catch(DBException& e) {
		close_tree(tree->get_name());
		throw e;
	}
}


/////////////////////////////////////////////////////////////////////

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
		for(auto& it : (*leaf_cache_r[key].first->get_childs())){
			it->item->second->set_file(nullptr);
			it->item->second = nullptr;
		}
		leaf_cache_r.erase(key);
	}
}

void forest::cache::check_intr_ref(string key)
{
	if(!intr_cache_r.count(key))
		return;
	if(intr_cache_r[key].second == 0 && !intr_cache.has(key)){
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

////////////FOREST_METHODS///////////////

void forest::create_root_file()
{
	Tree::seed(TREE_TYPES::KEY_STRING, ROOT_TREE);
}

void forest::open_root()
{
	FOREST = tree_ptr(new Tree(ROOT_TREE));
}

void forest::close_root()
{
	// Remove?
}

void forest::insert_tree(string name, string file_name)
{
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	/*
	file_data_t tmp(file_name.size(), [file_name](file_data_t* self, char* buf, int count){
		for(int i=0;i<count;i++){
			buf[i] = file_name[i];
		}
	});*/
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


