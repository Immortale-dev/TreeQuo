#include "db.h"

DB::DB()
{
	
}

DB::~DB()
{
	fold(false);
}

DB::DB(string path)
{
	bloom(path);
}

void DB::bloom(string path)
{
	if(blossomed)
		return;
		
	init_drivers();
		
	DBFS::root = path;
	if(!DBFS::exists(ROOT_TREE)){
		create_root_file();
	}
	
	FOREST = open_root();
	
	blossomed = true;
}

void DB::fold(bool cut=false)
{
	if(!blossomed)
		return;
		
	delete FOREST;
	blossomed = false;
}

void DB::create_qtree(TREE_TYPES type, string name)
{
	string file_name = create_qtree_base(type);
	//std::cout << file_name << std::endl;
	insert_qtree(name, file_name, type);
}

void DB::delete_qtree(string name)
{
	// TODO custom delete
}

DB::qtree_t DB::open_qtree(string name)
{
	// Error if not exists
	auto it = FOREST->find(name);
	if(it == FOREST->end()){
		throw DBException(DBException::ERRORS::TREE_DOES_NOT_EXISTS);
	}
	
	// Get tree path
	string path = it->second;
	qtree_t t;
	
	// Try to get from cache
	tree_cache_m.lock();
	if(tree_cache.has(path)){
		t = tree_cache.get(path);
		tree_cache_m.unlock();
		return t;
	}
	
	// Check for future
	if(tree_cache_q.count(path)){
		std::shared_future<qtree_t> f = tree_cache_q[path];
		tree_cache_m.unlock();
		return f.get();
	}
	
	// Create Promise
	std::promise<qtree_t> p;
	tree_cache_q[path] = p.get_future();
	tree_cache_m.unlock();
	
	// Get from file
	qtree_base_t tmp = read_base(path);
	t.type = tmp.type;
	switch(tmp.type){
		case TREE_TYPES::KEY_INT:
			t.tree = void_shared(new int_tree_t(tmp.factor, create_node<int_tree_t>(tmp.branch, tmp.is_leaf), tmp.count, driver_int));
			break;
		case TREE_TYPES::KEY_STRING:
			t.tree = void_shared(new string_tree_t(tmp.factor, create_node<string_tree_t>(tmp.branch, tmp.is_leaf), tmp.count, driver_string));
			break;
		default:
			throw DBException(DBException::ERRORS::NOT_VALID_TREE_TYPE);
	}
	
	// Fill in cache and send future event
	tree_cache_m.lock();
	tree_cache.push(path, t);
	p.set_value(t);
	tree_cache_q.erase(path);
	tree_cache_m.unlock();
	
	// return value
	return t;
}


/**
 * Root file contains: `${number_of_elements} ${tree_factor} ${type_of_tree} ${root_branch_file_name} ${is_root_branch_leaf_or_not} \n `
 * Leaf file contains: `${count_of_items} \n ${item_key} ${item_length} ${item_body} \n ... \n ${left_leaf_branch_name} \n ${right_leaf_branch_name} \n `
 * Intr file contains: `${is_children_branches_are_leafs_or_not} ${count_of_child_nodes} \n ${key1} ${key2} {key3} ... \n ${path0} ${path1} ${path2} ${path3} ... \n `
 * 
 **/ 
void DB::create_root_file()
{
	DBFS::File* rf = DBFS::create();
	if(rf->fail()){
		delete rf;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_ROOT);
		return;
	}
	rf->write("0\n-\n-\n");
	string root_branch_name = rf->name();
	rf->close();
	delete rf;
	
	DBFS::File* f = DBFS::create(ROOT_TREE);
	if(f->fail()){
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_ROOT);
		return;
	}
	f->write("0 " + std::to_string(DEFAULT_FACTOR) + " " + std::to_string((int)TREE_TYPES::ROOT) + " " + root_branch_name + " " + std::to_string((int)NODE_TYPES::LEAF) + "\n");
	f->close();
	delete f;
}

DB::string DB::create_qtree_base(TREE_TYPES type)
{
	string ret;
	DBFS::File* rf = DBFS::create();
	if(rf->fail()){
		delete rf;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_FILE);
		return "";
	}
	rf->write("0\n-\n-\n");
	string root_branch_name = rf->name();
	rf->close();
	delete rf;
	
	DBFS::File* f = DBFS::create();
	if(f->fail()){
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_FILE);
		return "";
	}
	ret = f->name();
	f->write("0 " + std::to_string(DEFAULT_FACTOR) + " " + std::to_string((int)type) + " " + root_branch_name + " " + std::to_string((int)NODE_TYPES::LEAF) + "\n");
	f->close();
	delete f;
	return ret;
}

void DB::insert_qtree(string name, string file_name, TREE_TYPES type)
{
	//std::cout << "HERE?" << std::endl;
	FOREST->insert(make_pair(name,file_name));
}

void DB::erase_qtree(string name)
{
	FOREST->erase(name);
}

DB::root_tree_t* DB::open_root()
{
	qtree_base_t base = read_base(ROOT_TREE);
	return new root_tree_t(base.factor, create_node<root_tree_t>(base.branch, base.is_leaf), base.count, driver_root);
}

DB::qtree_base_t DB::read_base(string filename)
{
	qtree_base_t ret;
	DBFS::File* f = new DBFS::File(filename);
	int t;
	f->read(ret.count);
	f->read(ret.factor);
	f->read(t);
	ret.type = (TREE_TYPES)t;
	f->read(ret.branch);
	f->read(ret.is_leaf);
	f->close();
	delete f;
	return ret;
}

void DB::init_drivers()
{
	driver_root = new driver_root_t(
		[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_enter<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_leave<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_insert<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_remove<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_reserve<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::node_ptr node, root_tree_t* tree){ this->d_release<root_tree_t>(node, tree); }
		,[this](typename root_tree_t::iterator it, typename root_tree_t::node_ptr node, int_t step, root_tree_t* tree){ this->d_before_move<root_tree_t>(it, node, step, tree); }
		,[this](typename root_tree_t::iterator it, typename root_tree_t::node_ptr node, int_t step, root_tree_t* tree){ this->d_after_move<root_tree_t>(it, node, step, tree); }
		,TREE_TYPES::ROOT
	);
	driver_int = new driver_int_t(
		[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_enter<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_leave<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_insert<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_remove<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_reserve<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::node_ptr node, int_tree_t* tree){ d_release<int_tree_t>(node, tree); }
		,[this](typename int_tree_t::iterator it, typename int_tree_t::node_ptr node, int_t step, int_tree_t* tree){ d_before_move<int_tree_t>(it, node, step, tree); }
		,[this](typename int_tree_t::iterator it, typename int_tree_t::node_ptr node, int_t step, int_tree_t* tree){ d_after_move<int_tree_t>(it, node, step, tree); }
		,TREE_TYPES::KEY_INT
	);
	driver_string = new driver_string_t(
		[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_enter<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_leave<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_insert<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_remove<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_reserve<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::node_ptr node, string_tree_t* tree){ d_release<string_tree_t>(node, tree); }
		,[this](typename string_tree_t::iterator it, typename string_tree_t::node_ptr node, int_t step, string_tree_t* tree){ d_before_move<string_tree_t>(it, node, step, tree); }
		,[this](typename string_tree_t::iterator it, typename string_tree_t::node_ptr node, int_t step, string_tree_t* tree){ d_after_move<string_tree_t>(it, node, step, tree); }
		,TREE_TYPES::KEY_STRING
	);
	//driver_root = new driver_root_t(d_enter<root_tree_t>, d_leave<root_tree_t>, d_insert<root_tree_t>, d_remove<root_tree_t>, d_reserve<root_tree_t>, d_release<root_tree_t>, d_before_move<root_tree_t>, d_after_move<root_tree_t>);
	//driver_int = new driver_int_t(d_enter<int_tree_t>, d_leave<int_tree_t>, d_insert<int_tree_t>, d_remove<int_tree_t>, d_reserve<int_tree_t>, d_release<int_tree_t>, d_before_move<int_tree_t>, d_after_move<int_tree_t>);
	//driver_string = new driver_string_t(d_enter<string_tree_t>, d_leave<string_tree_t>, d_insert<string_tree_t>, d_remove<string_tree_t>, d_reserve<string_tree_t>, d_release<string_tree_t>, d_before_move<string_tree_t>, d_after_move<string_tree_t>);
}



DB::qnode_data_ptr DB::create_node_data(bool ghost, string path)
{
	return qnode_data_ptr(new qnode_data_t(ghost, path));
}


