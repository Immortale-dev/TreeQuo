#include "forest.hpp"

namespace forest{
	Savior* savior;
}

void forest::bloom(string path)
{
	//=====//log_info_public("[forest::bloom] start");
	
	if(blossomed){
		log_info_public("[forest::bloom] end (already blossomed)");
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
	
	//=====//log_info_public("[forest::bloom] end");
}

void forest::fold()
{
	//=====//log_info_public("[forest::fold] start");
	
	if(!blossomed){
		//=====//log_info_public("[forest::fold] end (already folded)");
		return;
	}
		
	blossomed = false;
	
	delete savior;
	
	close_root();
	
	cache::release_cache();
	
	//=====//log_info_public("[forest::fold] end");
}

void forest::create_tree(TREE_TYPES type, string name, int factor)
{
	//=====//log_info_public("[forest::create_tree] start:"+name);
	
	if(!factor){
		factor = DEFAULT_FACTOR;
	}
	
	{
		if(FOREST->get_tree()->find(name) != FOREST->get_tree()->end()){
			//=====//log_info_details("[forest::create_tree] DBException::ERRORS::TREE_ALREADY_EXISTS");
			//=====//log_info_public("[forest::create_tree] end:"+name+" (errored)");
			throw DBException(DBException::ERRORS::TREE_ALREADY_EXISTS);
		}
	}
	
	//string file_name = Tree::seed(type, factor);
	string file_name = DBFS::random_filename();
	
	tree_ptr tree = tree_ptr(new Tree(file_name, type, factor));
	
	insert_tree(name, file_name, tree);
	
	//=====//log_info_public("[forest::create_tree] end:"+name);
}

void forest::delete_tree(string name)
{
	//=====//log_info_public("[forest::delete_tree] start:"+name);
	
	string path;
	// Not exist
	{
		auto it = FOREST->get_tree()->find(name);
		if(it == FOREST->get_tree()->end()){
			//=====//log_info_public("[forest::delete_tree] end:"+name+" (not exists)");
			return;
		}
		
		// Get tree path
		path = read_leaf_item(it->second);
	}
	
	//std::cout << "REMOVE_TREE_PATH: " + path + "\n";
	
	//std::cout << "ERASE TREE NAME: " + name + "\n";
	
	// Remove tree from forest
	//std::cout << "ERASE_FROM_FOREST\n";
	FOREST->erase(name);
	//std::cout << "ERASE_FROM_FOREST_END\n";
	
	//std::cout << "ERASE TREE START:::: " + path + "\n";
	
	// Erase tree
	erase_tree(path);
	
	//=====//log_info_public("[forest::delete_tree] end:" + name);
}

void forest::leave_tree(string path)
{
	//=====//log_info_public("[forest::leave_tree] start:"+path);
	cache::tree_cache_m.lock();
	cache::tree_cache_r[path].second--;
	cache::check_tree_ref(path);
	cache::tree_cache_m.unlock();
	//=====//log_info_public("[forest::leave_tree] end:"+path);
}

forest::tree_ptr forest::find_tree(string name)
{
	//=====//log_info_public("[forest::find_tree] start:"+name);
	
	// Error if not exists
	auto it = FOREST->get_tree()->find(name);
	if(it == FOREST->get_tree()->end()){
		//=====//log_info_public("[forest::find_tree] end:"+name+" (error:DBException::ERRORS::TREE_DOES_NOT_EXISTS)");
		throw DBException(DBException::ERRORS::TREE_DOES_NOT_EXISTS);
	}
	
	// Get tree path
	string path = read_leaf_item(it->second);
	
	//=====//log_info_public("[forest::find_tree] end:"+name);
	
	return open_tree(path);
}

forest::tree_ptr forest::open_tree(string path)
{
	//=====//log_info_public("[forest::open_tree] start:"+path);
	
	cache::tree_cache_m.lock();
	tree_ptr t = get_tree(path);
	cache::tree_cache_r[path].second++;
	cache::tree_cache_m.unlock();
	
	//=====//log_info_public("[forest::open_tree] end:"+path);
	
	return t;
}

void forest::insert_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{
	//=====//log_info_public("[forest::insert_leaf] start:"+name+" key:"+key);
	
	tree_ptr tree = find_tree(name);
	tree->insert(key, std::move(val));
	leave_tree(tree->get_name());
	
	//=====//log_info_public("[forest::insert_leaf] end:"+name+" key:"+key);
}

void forest::update_leaf(string name, tree_t::key_type key, tree_t::val_type val)
{
	//=====//log_info_public("[forest::update_leaf] start:"+name+" key:"+key);
	
	tree_ptr tree = find_tree(name);
	tree->insert(key, std::move(val), true);
	leave_tree(tree->get_name());
	
	//=====//log_info_public("[forest::update_leaf] end:"+name+" key:"+key);
}

void forest::erase_leaf(string name, tree_t::key_type key)
{
	//=====//log_info_public("[forest::erase_leaf] start:"+name+" key:"+key);
	
	tree_ptr tree = find_tree(name);
	tree->erase(key);
	leave_tree(tree->get_name());
	
	//=====//log_info_public("[forest::erase_leaf] end:"+name+" key:"+key);
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key)
{
	//=====//log_info_public("[forest::find_leaf] start:"+name+" key:"+key);
	
	tree_ptr tree = find_tree(name);
	try{
		tree_t::iterator t = tree->find(key);
		leave_tree(tree->get_name());
		
		//=====//log_info_public("[forest::find_leaf] end:"+name+" key:"+key);
		
		return LeafRecord_ptr(new LeafRecord(t, tree));
	} 
	catch(DBException& e) {
		//=====//log_info_public("[forest::find_leaf] end:"+name+" key:"+key + " (error)");
		
		leave_tree(tree->get_name());
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(string name, RECORD_POSITION position)
{
	//=====//log_info_public("[forest::find_leaf] start:"+name+" position:"+to_string((int)position));
	
	tree_ptr tree = find_tree(name);
	try{
		tree_t::iterator t;
		if(position == RECORD_POSITION::BEGIN){
			t = tree->get_tree()->begin();
		}
		else{
			t = --tree->get_tree()->end();
		}
		leave_tree(tree->get_name());
		
		//=====//log_info_public("[forest::find_leaf] end:"+name+" position:"+to_string((int)position));
		
		return LeafRecord_ptr(new LeafRecord(t, tree));
	}
	catch(DBException& e) {
		//=====//log_info_public("[forest::find_leaf] end:"+name+" position:"+to_string((int)position) + " (error)");
		
		leave_tree(tree->get_name());
		throw e;
	}
}

forest::LeafRecord_ptr forest::find_leaf(string name, tree_t::key_type key, RECORD_POSITION position)
{	
	if(position == RECORD_POSITION::BEGIN || position == RECORD_POSITION::END){
		return find_leaf(name, position);
	}
	
	//=====//log_info_public("[forest::find_leaf] start:"+name+" key:"+key+" position:"+to_string((int)position));
	
	tree_ptr tree = find_tree(name);
	try{
		tree_t::iterator t;
		if(position == RECORD_POSITION::LOWER){
			t = tree->get_tree()->lower_bound(key);
		}
		else {
			t = tree->get_tree()->upper_bound(key);
		}
		leave_tree(tree->get_name());
		
		//=====//log_info_public("[forest::find_leaf] end:"+name+" key:"+key+" position:"+to_string((int)position));
		
		return LeafRecord_ptr(new LeafRecord(t, tree));
	}
	catch(DBException& e) {
		//=====//log_info_public("[forest::find_leaf] end:"+name+" key:"+key+" position:"+to_string((int)position) + " (error)");
		
		leave_tree(tree->get_name());
		throw e;
	}
}


void forest::create_root_file()
{
	//=====//log_info_private("[forest::create_root_file] start");
	Tree::seed(TREE_TYPES::KEY_STRING, ROOT_TREE, ROOT_FACTOR);
	//=====//log_info_private("[forest::create_root_file] end");
}

void forest::open_root()
{
	//=====//log_info_private("[forest::open_root] start");
	FOREST = tree_ptr(new Tree(ROOT_TREE));
	//=====//log_info_private("[forest::open_root] end");
}

void forest::close_root()
{
	//=====//log_info_private("[forest::close_root] start");
	FOREST = nullptr;
	//=====//log_info_private("[forest::close_root] end");
	// Remove?
}

void forest::insert_tree(string name, string file_name, tree_ptr tree)
{
	//=====//log_info_private("[forest::insert_tree] start");
	cache::tree_cache_m.lock();
	cache::tree_cache.push(file_name, tree);
	cache::tree_cache_r[file_name] = make_pair(tree,1);
	cache::tree_cache_m.unlock();
	
	//std::cout << "SAVIOR_TREE " + file_name + "\n";
	savior->put(file_name, SAVE_TYPES::BASE, tree);
	
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, std::move(tmp));
	
	cache::tree_cache_m.lock();
	cache::tree_cache_r[file_name].second--;
	cache::check_tree_ref(file_name);
	cache::tree_cache_m.unlock();
	//file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	//FOREST->insert(name, tmp);
	//=====//log_info_private("[forest::insert_tree] end");
}

void forest::insert_tree(string name, string file_name)
{
	//REMOVE???
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, tmp);
}

void forest::erase_tree(string path)
{
	//=====//log_info_private("[forest::erase_tree] start");
	tree_ptr t = open_tree(path);
	
	t->get_tree()->clear();
	
	savior->remove(path, SAVE_TYPES::BASE, t);
	
	//DBFS::remove(path);

	// Clear cache
	cache::tree_cache_m.lock();
	//if(cache::tree_cache_r.count(path)){
	//	cache::tree_cache_r.erase(path);
	//}
	if(cache::tree_cache.has(path)){
		cache::tree_cache.remove(path);
	}
	cache::tree_cache_m.unlock();
	
	leave_tree(path);
	//=====//log_info_private("[forest::erase_tree] end");
}

forest::tree_ptr forest::get_tree(string path)
{
	return Tree::get(path);
}

forest::string forest::read_leaf_item(file_data_ptr item)
{
	//=====//log_info_public("[forest::read_leaf_item] start");
	
	int sz = item->size();
	char* buf = new char[sz];
	auto reader = item->get_reader();
	reader.read(buf,sz);
	string ret(buf,sz);
	delete[] buf;
	
	//=====//log_info_public("[forest::read_leaf_item] end");
	
	return ret;
}

