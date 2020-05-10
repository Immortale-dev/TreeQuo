#include "tree.hpp"

namespace forest{
	const string LEAF_NULL = "-";
}

forest::Tree::Tree(string path)
{
	// Read base tree file
	name = path;
	
	tree_base_read_t base = read_base(path);
	
	type = base.type;
	
	// Init BPT
	tree = new tree_t(base.factor, create_node(base.branch, base.branch_type), base.count, init_driver());
}

forest::Tree::~Tree()
{
	delete tree;
}

forest::string forest::Tree::seed(TREE_TYPES type, int factor)
{
	DBFS::File* f = DBFS::create();
	string path = f->name();
	seed_tree(f, type, factor);
	return path;
}

forest::string forest::Tree::seed(TREE_TYPES type, string path, int factor)
{	
	DBFS::File* f = DBFS::create(path);
	string path_name = f->name();
	seed_tree(f, type, factor);
	return path_name;
}

forest::tree_ptr forest::Tree::get(string path)
{
	tree_ptr t;
	
	// Try to get from cache
	if(cache::tree_cache.has(path)){
		t = cache::tree_cache.get(path);
		return t;
	}
	
	// Check reference
	if(cache::tree_cache_r.count(path)){
		t = cache::tree_cache_r[path].first;
		cache::tree_cache.push(path, t);
		return t;
	}
	
	/////////////////////////////////////////
	// TODO: POTENTIAL_RACE_CONDITION_HERE //
	/////////////////////////////////////////
	
	// Check for future
	if(cache::tree_cache_q.count(path)){
		std::shared_future<tree_ptr> f = cache::tree_cache_q[path];
		cache::tree_cache_m.unlock();
		t = f.get();
		cache::tree_cache_m.lock();
		return t;
	}
	
	// Create Promise
	std::promise<tree_ptr> p;
	cache::tree_cache_q[path] = p.get_future();
	cache::tree_cache_m.unlock();
	
	// Get from file
	t = tree_ptr(new Tree(path));
	
	// Fill in cache and send future event
	cache::tree_cache_m.lock();
	cache::tree_cache.push(path, t);
	cache::tree_cache_r[path] = std::make_pair(t,0);
	p.set_value(t);
	cache::tree_cache_q.erase(path);
	
	// return value
	return t;
}

forest::tree_t* forest::Tree::get_tree()
{
	return this->tree;
}

void forest::Tree::insert(tree_t::key_type key, tree_t::val_type val, bool update)
{
	tree->insert(make_pair(key, val), update);
	tree->save_base();
}

void forest::Tree::erase(tree_t::key_type key)
{
	tree->erase(key);
	tree->save_base();
}

forest::tree_t::iterator forest::Tree::find(tree_t::key_type key)
{
	auto it = tree->find(key);
	if(it == tree->end()){
		throw DBException(DBException::ERRORS::LEAF_DOES_NOT_EXISTS);
	}
	return it;
}


///////////////////////////////////////////////////////////////////////////


void forest::Tree::seed_tree(DBFS::File* f, TREE_TYPES type, int factor)
{
	if(f->fail()){
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_FILE);
		return;
	}
	f->write("0 " + to_string(factor) + " " + to_string((int)type) + " " + LEAF_NULL + " " + to_string((int)NODE_TYPES::LEAF) + "\n");
	f->close();
	delete f;
}


forest::Tree::tree_base_read_t forest::Tree::read_base(string filename)
{
	/*
	if(!DBFS::exists(filename)){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "FAIL WITH NOT EXISTS!!! " << filename << std::endl;
		throw "WTF???";
	}
	*/
	tree_base_read_t ret;
	DBFS::File* f = new DBFS::File(filename);
	/*
	if(f->fail()){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "FAIL WITH OPEN - OTHER!!! " << filename << std::endl;
		throw "WTF???";
	}
	*/
	int t;
	int lt;
	f->read(ret.count);
	f->read(ret.factor);
	f->read(t);
	ret.type = (TREE_TYPES)t;
	f->read(ret.branch);
	f->read(lt);
	ret.branch_type = NODE_TYPES(lt);
	/*
	if(f->fail()){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "FAIL WITH OPEN (READ_BASE)!!! " << filename << std::endl;
		throw "WTF???";
	}
	*/
	f->close();
	delete f;
	return ret;
}

forest::Tree::tree_intr_read_t forest::Tree::read_intr(string filename)
{
	using key_type = tree_t::key_type;
	
	int t;
	int c;
	
	DBFS::File* f = new DBFS::File(filename);
	f->read(t);
	f->read(c);
	
	std::vector<key_type>* keys = new std::vector<key_type>(c-1);
	std::vector<string>* vals = new std::vector<string>(c);
	
	for(int i=0;i<c-1;i++){
		f->read((*keys)[i]);
	}
	for(int i=0;i<c;i++){
		f->read((*vals)[i]);
	}
	
	f->close();
	delete f;
	
	tree_intr_read_t d;
	d.childs_type = (NODE_TYPES)t;
	d.child_keys = keys;
	d.child_values = vals;
	
	return d;
}

forest::Tree::tree_leaf_read_t forest::Tree::read_leaf(string filename)
{
	DBFS::File* f = new DBFS::File(filename);
	
	int c;
	string left_leaf, right_leaf;
	uint_t start_data;
	
	f->read(c);
	f->read(left_leaf);
	f->read(right_leaf);
	
	auto* keys = new std::vector<tree_t::key_type>(c);
	auto* vals_lengths = new std::vector<uint_t>(c);
	for(int i=0;i<c;i++){
		f->read((*keys)[i]);
	}
	for(int i=0;i<c;i++){
		f->read((*vals_lengths)[i]);
	}
	start_data = f->tell()+1;
	tree_leaf_read_t t;
	t.child_keys = keys;
	t.child_lengths = vals_lengths;
	t.left_leaf = left_leaf;
	t.right_leaf = right_leaf;
	t.start_data = start_data;
	t.file = f;
	return t;
}

void forest::Tree::materialize_intr(tree_t::node_ptr node)
{
	tree_t::node_ptr n;
	node_data_ptr data;
	
	node->data.owner_locks.m.lock();
	if(!has_data(node)){
		
		string temp_path = DBFS::random_filename();
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "TEMP_FILE_CREATED_INTR=" + temp_path + "\n";
		
		cache::intr_cache_m.lock();
		
		n = tree_t::node_ptr(new typename tree_t::InternalNode());
		data = create_node_data(false, temp_path);
		set_node_data(node, data);
		set_node_data(n, create_node_data(false, temp_path));
		
		cache::intr_cache_r[temp_path] = make_pair(n, 1);
		cache::intr_cache.push(temp_path, n);
		cache::intr_cache_m.unlock();
		
		node->data.owner_locks.m.unlock();
	}
	else {
		node->data.owner_locks.m.unlock();
		// Get node data
		data = get_node_data(node);
		string path = data->path;
		
		cache::intr_cache_m.lock();
		n = get_intr(path);
		cache::intr_cache_r[path].second++;
		cache::intr_cache_m.unlock();
		
	}
	
	if(node->data.travel_locks.wlock){
		lock_write(n);
	}
	else{
		lock_read(n);
	}
	
	// Owners lock made for readers-writer concept
	node->data.owner_locks.m.lock();
	int owners_count = node->data.owner_locks.c++;
	if(!owners_count){
		node->set_keys(n->get_keys());
		node->set_nodes(n->get_nodes());
		data->ghost = false;
	}
	node->data.owner_locks.m.unlock();
}

void forest::Tree::materialize_leaf(tree_t::node_ptr node)
{
	node->data.owner_locks.m.lock();
	
	tree_t::node_ptr n, next_leaf, prev_leaf;
	node_data_ptr data;
	
	if(!has_data(node)){
		
		string temp_path = DBFS::random_filename();
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "TEMP_FILE_CREATED_LEAF=" + temp_path + "\n";
		
		cache::leaf_cache_m.lock();
		
		n = tree_t::node_ptr(new typename tree_t::LeafNode());
		data = create_node_data(false, temp_path);
		set_node_data(node, data);
		set_node_data(n, create_node_data(false, temp_path));
		
		cache::leaf_cache_r[temp_path] = make_pair(n, 1);
		cache::leaf_cache.push(temp_path, n);
		cache::leaf_cache_m.unlock();
		
		node->data.owner_locks.m.unlock();
	}
	else{
		node->data.owner_locks.m.unlock();
	
		// Get node data
		data = get_node_data(node);
		string path = data->path;
		
		cache::leaf_cache_m.lock();
		n = get_leaf(path);
		cache::leaf_cache_r[path].second++;
		cache::leaf_cache_m.unlock();
		
	}
	
	// Lock original node as well
	if(node->data.travel_locks.wlock){
		lock_write(n);
	}
	else{
		lock_read(n);
	}
	
	// Own lock made for readers-writer concept to not
	// overwrite the data
	node->data.owner_locks.m.lock();
	int own_count = node->data.owner_locks.c++;
	if(own_count){
		node->data.owner_locks.m.unlock();
		return;
	}
		
	node->set_childs(n->get_childs());
	
	next_leaf = tree_t::node_ptr(new tree_t::LeafNode());
	prev_leaf = tree_t::node_ptr(new tree_t::LeafNode());
	
	node_data_ptr ndata = get_node_data(n);
	
	set_node_data(next_leaf, create_node_data(true, ndata->next));
	set_node_data(prev_leaf, create_node_data(true, ndata->prev));
	
	if(ndata->prev != LEAF_NULL){
		node->set_prev_leaf(prev_leaf);
	}
	if(ndata->next != LEAF_NULL){
		node->set_next_leaf(next_leaf);
	}
	data->ghost = false;
	
	// Set all the data and unlock owning
	node->data.owner_locks.m.unlock();
}

void forest::Tree::unmaterialize_intr(tree_t::node_ptr node)
{
	if(!has_data(node)){
		assert(false);
		node->data.owner_locks.m.lock();
		node->data.owner_locks.c--;
		node->data.owner_locks.m.unlock();
		return;
	}
	
	// Get node data
	node_data_ptr data = get_node_data(node);
	string path = data->path;
	
	// Unlock original node if it was not already deleted
	cache::intr_cache_m.lock();
	assert(cache::intr_cache_r[path].second > 0);
	assert(cache::intr_cache_r.count(path));
	if(cache::intr_cache_r.count(path)){
		tree_t::node_ptr n = cache::intr_cache_r[path].first;
		if(node->data.travel_locks.wlock){
			unlock_write(n);
		}
		else{
			unlock_read(n);
		}
	}
	cache::intr_cache_m.unlock();
	
	// made for readers-writer concept
	node->data.owner_locks.m.lock();
	int owners_count = --node->data.owner_locks.c;
	assert(owners_count >= 0);
	if(!owners_count){
		node->set_keys(nullptr);
		node->set_nodes(nullptr);
		data->ghost = true;
	}
	node->data.owner_locks.m.unlock();
	
	cache::intr_cache_m.lock();
	cache::intr_cache_r[path].second--;
	cache::check_intr_ref(path);
	cache::intr_cache_m.unlock();
}

void forest::Tree::unmaterialize_leaf(tree_t::node_ptr node)
{
	if(!has_data(node)){
		assert(false);
		node->data.owner_locks.m.lock();
		node->data.owner_locks.c--;
		node->data.owner_locks.m.unlock();
		return;
	}
	
	// Get node data
	node_data_ptr data = get_node_data(node);
	string path = data->path;
	
	// Unlock original node if it was not already deleted
	cache::leaf_cache_m.lock();
	assert(cache::leaf_cache_r.count(path));
	assert(cache::leaf_cache_r[path].second > 0);
	if(cache::leaf_cache_r.count(path)){
		tree_t::node_ptr n = cache::leaf_cache_r[path].first;
		if(node->data.travel_locks.wlock){
			unlock_write(n);
		}
		else{
			unlock_read(n);
		}
	}
	cache::leaf_cache_m.unlock();
	
	// Owner lock made for readers-writer concept to not
	// overwrite the data
	node->data.owner_locks.m.lock();
	int own_count = --node->data.owner_locks.c;
	if(own_count == 0){
		node->set_childs(nullptr);
		node->set_prev_leaf(nullptr);
		node->set_next_leaf(nullptr);
		data->ghost = true;
	}
	node->data.owner_locks.m.unlock();
	
	cache::leaf_cache_m.lock();
	cache::leaf_cache_r[path].second--;
	cache::check_leaf_ref(path);
	cache::leaf_cache_m.unlock();
}

forest::node_ptr forest::Tree::create_node(string path, NODE_TYPES node_type)
{
	tree_t::Node* node;
	if(node_type == NODE_TYPES::INTR){
		node = new tree_t::InternalNode();
	} else {
		node = new tree_t::LeafNode();
	}
	if(path != LEAF_NULL){
		set_node_data(node, create_node_data(true, path));
	}
	return node_ptr(node);
}

forest::string forest::Tree::get_name()
{
	return name;
}

forest::TREE_TYPES forest::Tree::get_type()
{
	return type;
}

forest::Tree::node_data_ptr forest::Tree::create_node_data(bool ghost, string path)
{
	return node_data_ptr(new node_data_t(ghost, path));
}

forest::Tree::node_data_ptr forest::Tree::create_node_data(bool ghost, string path, string prev, string next)
{
	auto p = node_data_ptr(new node_data_t(ghost, path));
	p->prev = prev;
	p->next = next;
	return p;
}

forest::Tree::node_data_ptr forest::Tree::get_node_data(tree_t::node_ptr node)
{
	return std::static_pointer_cast<node_data_t>(node->data.drive_data);
}

void forest::Tree::set_node_data(tree_t::node_ptr node, node_data_ptr d)
{
	set_node_data(node.get(), d);
}

void forest::Tree::set_node_data(tree_t::Node* node, node_data_ptr d)
{
	node->data.drive_data = d;
}

bool forest::Tree::has_data(tree_t::node_ptr node)
{
	return has_data(node.get());
}

bool forest::Tree::has_data(tree_t::Node* node)
{
	return (bool)(node->data.drive_data);
}

forest::tree_t::node_ptr forest::Tree::get_intr(string path)
{	
	node_ptr intr_data;
	
	// Check cache
	if(cache::intr_cache.has(path)){
		intr_data = cache::intr_cache.get(path);
		return intr_data;
	}
	
	// Check reference
	if(cache::intr_cache_r.count(path)){
		intr_data = cache::intr_cache_r[path].first;
		cache::intr_cache.push(path, intr_data);
		return intr_data;
	}
	
	// Create and lock node
	intr_data = node_ptr(new tree_t::InternalNode());
	lock_write(intr_data);
	
	// Put it into the cache
	cache::intr_cache_r[path] = std::make_pair(intr_data,1);
	cache::intr_cache_m.unlock();
	
	// Fill node
	tree_intr_read_t intr_d = read_intr(path);
	std::vector<tree_t::key_type>* keys_ptr = intr_d.child_keys;
	std::vector<string>* vals_ptr = intr_d.child_values;
	intr_data->add_keys(0, keys_ptr->begin(), keys_ptr->end());
	int c = vals_ptr->size();
	for(int i=0;i<c;i++){
		node_ptr n;
		string& child_path = (*vals_ptr)[i];
		if(intr_d.childs_type == NODE_TYPES::INTR){
			n = node_ptr(new typename tree_t::InternalNode());
		} else {
			n = node_ptr(new typename tree_t::LeafNode());
		}
		set_node_data(n, create_node_data(true, child_path));
		intr_data->add_nodes(i,n);
	}
	set_node_data(intr_data, create_node_data(false, path));
	
	// Unlock node
	cache::intr_cache_m.lock();
	if(!cache::intr_cache.has(path)){
		cache::intr_cache.push(path, intr_data);
	}
	cache::intr_cache_r[path].second--;
	unlock_write(intr_data);
	
	// Return
	return intr_data;
}

forest::tree_t::node_ptr forest::Tree::get_leaf(string path)
{
	using entry_pair = std::pair<const tree_t::key_type, tree_t::val_type>;
	using entry_ptr = std::shared_ptr<entry_pair>;
	
	node_ptr leaf_data;
	
	// Check cache
	if(cache::leaf_cache.has(path)){
		leaf_data = cache::leaf_cache.get(path);
		return leaf_data;
	}
	
	// Check reference
	if(cache::leaf_cache_r.count(path)){
		leaf_data = cache::leaf_cache_r[path].first;
		cache::leaf_cache.push(path, leaf_data);
		return leaf_data;
	}

	// Create and lock node
	leaf_data = node_ptr(new typename tree_t::LeafNode());
	lock_write(leaf_data);
	leaf_data->data.change_locks.m.lock();
	
	// Put it into the cache
	cache::leaf_cache_r[path] = std::make_pair(leaf_data,1);
	cache::leaf_cache_m.unlock();
	
	// Fill data
	tree_leaf_read_t leaf_d = read_leaf(path);
	std::vector<tree_t::key_type>* keys_ptr = leaf_d.child_keys;
	std::vector<uint_t>* vals_length = leaf_d.child_lengths;
	uint_t start_data = leaf_d.start_data;
	std::shared_ptr<DBFS::File> f(leaf_d.file);
	int c = keys_ptr->size();
	uint_t last_len = 0;
	for(int i=0;i<c;i++){
		leaf_data->insert(entry_ptr(new entry_pair( (*keys_ptr)[i], file_data_ptr(new file_data_t(f, start_data+last_len, (*vals_length)[i])) )));
		last_len += (*vals_length)[i];
	}
	
	int childs_size = leaf_data->childs_size();
	auto childs = leaf_data->get_childs();
	for(int i=0;i<childs_size;i++){
		(*childs)[i]->node = leaf_data;
		(*childs)[i]->pos = i;
	}
	
	set_node_data(leaf_data, create_node_data(false, path, leaf_d.left_leaf, leaf_d.right_leaf));
	
	// Unlock node and push to cache
	cache::leaf_cache_m.lock();
	if(!cache::leaf_cache.has(path)){
		cache::leaf_cache.push(path, leaf_data);
	}
	assert(cache::leaf_cache_r[path].second > 0);
	cache::leaf_cache_r[path].second--;
	leaf_data->data.change_locks.m.unlock();
	unlock_write(leaf_data);
	
	// Return
	return leaf_data;
}

void forest::Tree::write_intr(DBFS::File* file, tree_intr_read_t data)
{
	auto* keys = data.child_keys;
	auto* paths = data.child_values;
	file->write( to_string((int)data.childs_type) + " " + to_string(paths->size()) + "\n" );
	
	string valsStr = "";
	std::stringstream ss;
	for(auto& key : (*keys)){
		ss << key << " ";
	}
	for(auto& val : (*paths)){
		valsStr.append(val + " ");
	}
	file->write(ss.str() + "\n");
	file->write(valsStr);
}

void forest::Tree::write_base(DBFS::File* file, tree_base_read_t data)
{
	file->write( to_string(data.count) + " " + to_string(data.factor) + " " + to_string((int)data.type) + " " + data.branch + " " + to_string((int)data.branch_type) + "\n" );
}

void forest::Tree::write_leaf(std::shared_ptr<DBFS::File> file, tree_leaf_read_t data)
{
	auto* keys = data.child_keys;
	auto* lengths = data.child_lengths;
	int c = keys->size();
	file->write(to_string(c) + " " + data.left_leaf + " " + data.right_leaf+ "\n");
	string lenStr = "";
	std::stringstream ss;
	for(auto& key : (*keys)){
		ss << key << " ";
	}
	for(auto& len : (*lengths)){
		lenStr.append(to_string(len) + " ");
	}
	if(lenStr.size()){
		lenStr.pop_back();
	}
	file->write(ss.str()+"\n");
	file->write(lenStr+"\n");
}

void forest::Tree::write_leaf_item(std::shared_ptr<DBFS::File> file, tree_t::val_type& data)
{
	int_t start_data = file->tell();
	
	int read_size = 4*1024;
	char* buf = new char[read_size];
	int rsz;
	auto reader = data->get_reader();
	
	try{
		while( (rsz = reader.read(buf, read_size)) ){
			file->write(buf, rsz);
		}
	} catch(...){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ERROR WHEN WRITE LEAF ITEM\n";
		throw "error";
	}
	
	delete[] buf;
	data->set_start(start_data);
	data->set_file(file);
	data->delete_cache();
}

// LOCKERS //

void forest::Tree::lock_read(tree_t::node_ptr node)
{
	lock_read(node.get());
}

void forest::Tree::lock_read(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.lock();
	//return;
	
	tl.m.lock();
	if(tl.c++ == 0){
		tl.g.lock();
	}
	tl.m.unlock();
}

void forest::Tree::unlock_read(tree_t::node_ptr node)
{
	unlock_read(node.get());
}

void forest::Tree::unlock_read(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.unlock();
	//return;
	
	tl.m.lock();
	if(--tl.c == 0){
		tl.g.unlock();
	}
	tl.m.unlock();
}

void forest::Tree::lock_write(tree_t::node_ptr node)
{
	lock_write(node.get());
}

void forest::Tree::lock_write(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.lock();
	//return;
	
	tl.g.lock();
	tl.wlock = true;
}

void forest::Tree::unlock_write(tree_t::node_ptr node)
{
	unlock_write(node.get());
}

void forest::Tree::unlock_write(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.unlock();
	//return;
	
	tl.wlock = false;
	tl.g.unlock();
}

void forest::Tree::lock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type)
{
	lock_type(node.get(), type);
}

void forest::Tree::lock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? lock_write(node) : lock_read(node);
}

void forest::Tree::unlock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type)
{
	unlock_type(node.get(), type);
}

void forest::Tree::unlock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? unlock_write(node) : unlock_read(node);
}

void forest::Tree::clear_node_cache(tree_t::node_ptr node)
{
	node_data_ptr data = get_node_data(node);
	string path = data->path;
	if(node->is_leaf()){
		cache::leaf_cache_m.lock();
		if(cache::leaf_cache_r.count(path)){
			cache::leaf_cache_r.erase(path);
		}
		if(cache::leaf_cache.has(path)){
			cache::leaf_cache.remove(path);
		}
		cache::leaf_cache_m.unlock();
	}
	else{
		cache::intr_cache_m.lock();
		if(cache::intr_cache_r.count(path)){
			cache::intr_cache_r.erase(path);
		}
		if(cache::intr_cache.has(path)){
			cache::intr_cache.remove(path);
		}
		cache::intr_cache_m.unlock();
	}
}

forest::driver_t* forest::Tree::init_driver()
{
	return new driver_t(
		[this](tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_enter(node, type, tree); }
		,[this](tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_leave(node, type, tree); }
		,[this](tree_t::node_ptr node, tree_t* tree){ this->d_insert(node, tree); }
		,[this](tree_t::node_ptr node, tree_t* tree){ this->d_remove(node, tree); }
		,[this](tree_t::child_item_type_ptr item, int_t step, tree_t* tree){ this->d_before_move(item, step, tree); }
		,[this](tree_t::child_item_type_ptr item, int_t step, tree_t* tree){ this->d_after_move(item, step, tree); }
		,[this](tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_item_reserve(item, type, tree); }
		,[this](tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_item_release(item, type, tree); }
		,[this](tree_t::node_ptr node, bool release, tree_t* tree){ this->d_item_move(node, release, tree); }
		,[this](tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_reserve(node, type, tree); }
		,[this](tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree){ this->d_release(node, type, tree); }
		,[this](tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree){ this->d_leaf_insert(node, item, tree); }
		,[this](tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree){ this->d_leaf_delete(node, item, tree); }
		,[this](tree_t::node_ptr node, tree_t::node_ptr new_node, tree_t::node_ptr link_node, tree_t* tree){ this->d_leaf_split(node, new_node, link_node, tree); }
		,[this](tree_t::node_ptr node, tree_t::node_ptr join_node, tree_t::node_ptr link_node, tree_t* tree){ this->d_leaf_join(node, join_node, link_node, tree); }
		,[this](tree_t::node_ptr node, tree_t::node_ptr shift_node, tree_t* tree){ this->d_leaf_shift(node, shift_node, tree); }
		,[this](tree_t::node_ptr node, tree_t* tree){ this->d_leaf_free(node, tree); }
		,[this](tree_t::node_ptr node, tree_t::node_ptr ref_node, tree_t::LEAF_REF ref, tree_t* tree){ this->d_leaf_ref(node, ref_node, ref, tree); }
		,[this](tree_t::node_ptr node, tree_t* tree){ this->d_save_base(node, tree); }
	);
}



// Proceed
void forest::Tree::d_enter(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ENTER_START\n";
		
	// Lock node mutex
	lock_type(node, type);
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ENTER_END - STEM\n";
		return;
	}
		
	if(!node->is_leaf()){
		materialize_intr(node);
	}
	else{
		materialize_leaf(node);
	}
	
	if(node->is_leaf()){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "--ENTER_LEAF-" + get_node_data(node)->path + "\n";
	} else {
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "--ENTER_INTR-" + get_node_data(node)->path + "\n";
	}
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ENTER_END\n";
}

void forest::Tree::d_leave(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "LEAVE_START\n";
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		unlock_type(node, type);
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "LEAVE_END - STEM\n";
		return;
	}
	
	if(node->is_leaf()){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "--LEAVE_LEAF-" + get_node_data(node)->path + "\n";
	} else {
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "--LEAVE_INTR-" + get_node_data(node)->path + "\n";
	}
	
	if(!node->is_leaf()){
		unmaterialize_intr(node);
	}
	else{
		unmaterialize_leaf(node);
	}
	unlock_type(node, type);
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "LEAVE_END\n";
}

void forest::Tree::d_insert(tree_t::node_ptr node, tree_t* tree)
{
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "INSERT_START" << std::endl;
	
	if(tree->is_stem_pub(node)){
		assert(false);
	}
	
	DBFS::File* f = DBFS::create();
	string new_name = f->name();
	
	if(!node->is_leaf()){
		tree_intr_read_t intr_d;
		intr_d.childs_type = ((node->first_child_node()->is_leaf()) ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
		int c = node->get_nodes()->size();
		auto* keys = new std::vector<tree_t::key_type>(node->keys_iterator(), node->keys_iterator_end());
		auto* nodes = new std::vector<string>(c);
		for(int i=0;i<c;i++){
			node_data_ptr d = get_node_data( (*(node->get_nodes()))[i] );
			(*nodes)[i] = d->path;
		}
		intr_d.child_keys = keys;
		intr_d.child_values = nodes;
		write_intr(f, intr_d);
		
		f->close();
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		
		DBFS::remove(cur_name);
		DBFS::move(new_name, cur_name);
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "SAVED_FILE_INTR=" + cur_name + "\n";
	} else {
		tree_leaf_read_t leaf_d;
		auto* keys = new std::vector<tree_t::key_type>();
		auto* lengths = new std::vector<uint_t>();
		
		node_ptr p_leaf = node->prev_leaf();
		node_ptr n_leaf = node->next_leaf();
		
		if(p_leaf){
			assert(has_data(p_leaf));
			node_data_ptr p_leaf_data = get_node_data(p_leaf);
			leaf_d.left_leaf = p_leaf_data->path;
		} else {
			leaf_d.left_leaf = LEAF_NULL;
		}
		if(n_leaf){
			assert(has_data(n_leaf));
			node_data_ptr n_leaf_data = get_node_data(n_leaf);
			leaf_d.right_leaf = n_leaf_data->path;
		} else {
			leaf_d.right_leaf = LEAF_NULL;
		}
		
		for(auto& it : *(node->get_childs()) ){
			keys->push_back(it->item->first);
			lengths->push_back(it->item->second->size());
		}
		
		leaf_d.child_keys = keys;
		leaf_d.child_lengths = lengths;
		std::shared_ptr<DBFS::File> fp(f);
		write_leaf(fp, leaf_d);
		
		auto lock = fp->get_lock();
		
		for(auto& it : *(node->get_childs()) ){
			write_leaf_item(fp, it->item->second);
		}
		
		fp->stream().flush();
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		
		cache::leaf_cache_m.lock();
		
		assert(cache::leaf_cache_r.count(cur_name));
		assert(cache::leaf_cache_r[cur_name].second > 0);

		node_ptr n = get_leaf(cur_name);
		
		assert(n->get_childs() == node->get_childs());
		
		///node_data_ptr c_data = get_node_data(n);
		///c_data->prev = leaf_d.left_leaf;
		///c_data->next = leaf_d.right_leaf;
		
		cache::leaf_cache_m.unlock();
		
		DBFS::remove(cur_name);
		fp->move(cur_name);
		
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "SAVED_FILE_LEAF=" + cur_name + "\n";
	}
	
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "INSERT_END" << std::endl;
}

void forest::Tree::d_remove(tree_t::node_ptr node, tree_t* tree)
{
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "REMOVE_START" << std::endl;
	
	if(!has_data(node)){
		////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "REMOVE_END - NODATA" << std::endl;
		return;
	}
	
	node_data_ptr data = get_node_data(node);
	
	// TODO: RECODE WHEN DELAYED SAVE IMPLEMENTED
	if(node->is_leaf() && node->size()){
		node->first_child()->item->second->file->close();
		node->get_childs()->resize(0);
	}
	
	DBFS::remove(data->path);
	
	///clear_node_cache(node);
	
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "REMOVE_END" << std::endl;
}

void forest::Tree::d_reserve(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{	
	assert(has_data(node));
	
	string path = get_node_data(node)->path;
	
	cache::leaf_cache_m.lock();
	node = get_leaf(path);
	cache::leaf_cache_r[path].second++;
	cache::leaf_cache_m.unlock();
	
	auto& ch_node = node->data.change_locks;
	if(type == tree_t::PROCESS_TYPE::READ){
		ch_node.g.lock();
		if(ch_node.c++ == 0){
			ch_node.m.lock();
		}
		ch_node.g.unlock();
	} else {
		ch_node.m.lock();
	}
}

void forest::Tree::d_release(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{	
	assert(has_data(node));
	
	string path = get_node_data(node)->path;
	
	cache::leaf_cache_m.lock();
	node = get_leaf(path);
	cache::leaf_cache_r[path].second--;
	cache::check_leaf_ref(path);
	cache::leaf_cache_m.unlock();
	
	auto& ch_node = node->data.change_locks;
	if(type == tree_t::PROCESS_TYPE::READ){
		ch_node.g.lock();
		if(--ch_node.c == 0){
			ch_node.m.unlock();
		}
		ch_node.g.unlock();
	} else {
		ch_node.m.unlock();
	}
}

void forest::Tree::d_before_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move Start\n";

	if(!item){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move END - NOTITEM\n";
		return;
	}
	
	tree_t::node_ptr node;
	
	// Make sure to lock the right node
	do{	
		
		cache::leaf_cache_m.lock();
		
		item->item->second->o.lock();
		node = item->node;
		item->item->second->o.unlock();
		
		assert(bool(node));
		assert(has_data(node));
		
		string path = get_node_data(node)->path;
		
		node = get_leaf(path);
		
		cache::leaf_cache_m.unlock();
		
		auto& ch_node = node->data.change_locks;
		
		{
			// Check for priority
			std::unique_lock<std::mutex> plock(ch_node.p);
			while(ch_node.shared_lock && !item->item->second->shared_lock){
				ch_node.cond.wait(plock);
			}
		}
		
		
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move before first lock\n";
		std::unique_lock<std::mutex> lock(ch_node.g);
		////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock\n";
		
		/*
		// Make sure this node is not priored
		if(ch_node.shared_lock && *ch_node.shared_lock){
			ch_node.cond.wait(lock);
		}
		*/
		
		////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after wait\n";
		
		// Readers-writer lock
		if(ch_node.c++ == 0){
			//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_READ-" + path + "\n";
			ch_node.m.lock();
		}
		
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock m\n";
		
		// If it is still the same node - break the loop
		if(path == get_node_data(item->node)->path){
			break;
		}
		
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move retry\n";
		
		// Readers-writer unlock
		if(--ch_node.c == 0){
			//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_CHANGE_READ-" + path + "\n";
			ch_node.m.unlock();
		}
		
	}while(true);
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move MIDDLE\n";
	
	//auto& ch_node = node->data.change_locks;
	
	cache::leaf_cache_m.lock();
	assert(cache::leaf_cache_r[get_node_data(node)->path].second > 0);
	cache::leaf_cache_r[get_node_data(node)->path].second++;
	cache::leaf_cache_m.unlock();
	
	if( (step < 0 && item->pos == 0) || (step > 0 && item->pos+1 == node->childs_size()) ){
		
		tree_t::node_ptr new_node;
		
		string new_path = (step > 0) ? get_node_data(node)->next : get_node_data(node)->prev;
		
		// If no next node - break
		if(new_path != LEAF_NULL){
			
			// Reserve node (anti rc)
			cache::leaf_cache_m.lock();
			new_node = get_leaf(new_path);
			assert((bool)new_node);
			cache::leaf_cache_r[new_path].second++;
			cache::leaf_cache_m.unlock();
			
			auto& ch_new_node = new_node->data.change_locks;
			
			// Start read lock
			//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move before second lock\n";
			std::unique_lock<std::mutex> lock(ch_new_node.g);
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after second lock\n";
			
			// Make sure new node is not priored
			/*if( 
				(ch_new_node.shared_lock && *ch_new_node.shared_lock)
				&& (ch_node.shared_lock && *ch_node.shared_lock) 
				&& (ch_new_node.shared_lock.get() != ch_node.shared_lock.get()) 
			){
				ch_new_node.cond.wait(lock);
			}*/
			
			// Complete read lock
			if(ch_new_node.c++ == 0){
				//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_READ-" + new_path + "\n";
				ch_new_node.m.lock();
			}
			
			node->data.owner_locks.m.lock();
			if(step > 0){
				if(!node->next_leaf() || node->next_leaf().get() != new_node.get())
					node->set_next_leaf(new_node);
			}
			else{
				if(!node->prev_leaf() || node->prev_leaf().get() != new_node.get())
					node->set_prev_leaf(new_node);
			}
			node->data.owner_locks.m.unlock();
		} else {
			node->data.owner_locks.m.lock();
			if(step > 0){
				if(node->next_leaf())
					node->set_next_leaf(nullptr);
			}
			else{
				if(node->prev_leaf())
					node->set_prev_leaf(nullptr);
			}
			node->data.owner_locks.m.unlock();
		}
		
		/*
		// Read unlock previous node
		ch_node.g.lock();
		if(--ch_node.c == 0){
			ch_node.m.unlock();
		}
		ch_node.g.unlock();
		*/
	}

	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move END\n";
}

void forest::Tree::d_after_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "After Move START\n";
	
	if(!item || !step){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "After Move END - NOTITEM\n";
		return;
	}
	
	cache::leaf_cache_m.lock();
	
	item->item->second->o.lock();
	tree_t::node_ptr node = item->node;
	item->item->second->o.unlock();
	
	tree_t::node_ptr node_old;
	
	assert((bool)node);
	assert(has_data(node));
	
	string path = get_node_data(node)->path;
	string path_old;
	
	node = get_leaf(path);
	
	if( (step > 0 && item->pos == 0) || (step < 0 && item->pos+1 == node->childs_size()) ){
		path_old = (step > 0) ? get_node_data(node)->prev : get_node_data(node)->next;
		if(path_old != LEAF_NULL){
			node_old = get_leaf(path_old);
		}
	}
	//cache::leaf_cache_m.unlock();
	
	//cache::leaf_cache_m.lock();
	assert(cache::leaf_cache_r.count(path));
	assert(cache::leaf_cache_r[path].second > 1);
	
	if(node_old){
		assert(cache::leaf_cache_r.count(path_old));
		assert(cache::leaf_cache_r[path_old].second > 0);
	}
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-AFTER_NODE_RESERVED-" + path + "\n";
	
	if(node_old){
		cache::leaf_cache_r[path_old].second--;
		cache::check_leaf_ref(path_old);
	}
	
	cache::leaf_cache_r[path].second--;
	cache::check_leaf_ref(path);
		
	cache::leaf_cache_m.unlock();
	
	if(node_old){
		// Unlock old node
		auto& ch_node_old = node_old->data.change_locks;
		ch_node_old.g.lock();
		if(--ch_node_old.c == 0){
			//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_CHANGE_READ-" + path + "\n";
			ch_node_old.m.unlock();
		}
		ch_node_old.g.unlock();
	}
	
	// unlock curr node
	auto& ch_node = node->data.change_locks;
	ch_node.g.lock();
	if(--ch_node.c == 0){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_CHANGE_READ-" + path + "\n";
		ch_node.m.unlock();
	}
	ch_node.g.unlock();
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "After Move END\n";
}

void forest::Tree::d_item_reserve(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ITER_RESERVE_START\n";
	
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_cache_m.lock();
		node = item->node;
		path = get_node_data(node)->path;
		node = get_leaf(path);
		cache::leaf_cache_m.unlock();
	}
	else{
		// Make sure to lock the right node
		
		bool wasret = false;
		
		do{	
			cache::leaf_cache_m.lock();
			
			item->item->second->o.lock();
			node = item->node;
			item->item->second->o.unlock();
			
			if(wasret && !node){
				//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ERR: " + path + "\n";
				assert((bool)node);
			}
			
			if(!node){
				//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-assert_error-" + item->item->first + "\n";
				assert(false);
			}
			assert(has_data(node));
			
			path = get_node_data(node)->path;
			
			node = get_leaf(path);
			cache::leaf_cache_m.unlock();
			
			auto& ch_node = node->data.change_locks;
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move before first lock" << std::endl;
			std::unique_lock<std::mutex> lock(ch_node.g);
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock" << std::endl;
			
			/*
			// Make sure this node is not priored
			if(ch_node.shared_lock && *ch_node.shared_lock){
				ch_node.cond.wait(lock);
			}
			*/
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after wait" << std::endl;
			
			// Readers-writer lock
			if(ch_node.c++ == 0){
				ch_node.m.lock();
			}
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock m" << std::endl;
			
			// If it is still the same node - break the loop
			if(path == get_node_data(item->node)->path){
				break;
			}
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move retry" << std::endl;
			
			// Readers-writer unlock
			if(--ch_node.c == 0){
				ch_node.m.unlock();
			}
			
			wasret = true;
			//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "RESERVE_RETRY\n";
			
		}while(true);
	}
	
	cache::leaf_cache_m.lock();
	assert(cache::leaf_cache_r.count(path));
	assert(cache::leaf_cache_r[path].second > 0);
	
	cache::leaf_cache_r[path].second++;
	
	//===std::cout << convert_to_string(std::this_thread::get_id()) + "--RSRV--Reserve-" + path + "-" + std::to_string(item->pos) + "\n";
	
	cache::insert_item(path, item->pos);
	cache::leaf_cache_m.unlock();
	
	//item->item->second->m.lock();
	//return;
	
	// Lock item
	auto it = item->item->second;
	if(type == tree_t::PROCESS_TYPE::WRITE){
		it->m.lock();
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_ITEM_WRITE-" + item->item->first + "\n";
	}
	else{
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_ITEM_READ-" + item->item->first + "\n";
		it->g.lock();
		if(it->c++ == 0){
			it->m.lock();
		}
		it->g.unlock();
	}
	
	auto& ch_node = node->data.change_locks;
	if(type == tree_t::PROCESS_TYPE::READ){
		ch_node.g.lock();
		if(--ch_node.c == 0){
			ch_node.m.unlock();
		}
		ch_node.g.unlock();
	}
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ITER_RESERVE_END\n";
}

void forest::Tree::d_item_release(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ITEM_RELEASE_START\n";
	
	/////////////////////////////////
	//TODO: IF RC HAPPEND LOOK HERE//
	/////////////////////////////////
	
	
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_cache_m.lock();
		node = item->node;
		path = get_node_data(node)->path;
		node = get_leaf(path);
		cache::leaf_cache_m.unlock();
	}
	else{
		// Make sure to lock the right node
		do{	
			cache::leaf_cache_m.lock();
			
			item->item->second->o.lock();
			node = item->node;
			item->item->second->o.unlock();
			
			assert(bool(node));
			assert(has_data(node));
			
			path = get_node_data(node)->path;
			
			node = get_leaf(path);
			cache::leaf_cache_m.unlock();
			
			auto& ch_node = node->data.change_locks;
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move before first lock" << std::endl;
			std::unique_lock<std::mutex> lock(ch_node.g);
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock" << std::endl;
			
			/*
			// Make sure this node is not priored
			if(ch_node.shared_lock && *ch_node.shared_lock){
				ch_node.cond.wait(lock);
			}
			*/
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after wait" << std::endl;
			
			// Readers-writer lock
			if(ch_node.c++ == 0){
				ch_node.m.lock();
			}
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move after first lock m" << std::endl;
			
			// If it is still the same node - break the loop
			if(path == get_node_data(item->node)->path){
				break;
			}
			
			////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "Before Move retry" << std::endl;
			
			// Readers-writer unlock
			if(--ch_node.c == 0){
				ch_node.m.unlock();
			}
		}while(true);
	}
	
	
	cache::leaf_cache_m.lock();
	
	if(type == tree_t::PROCESS_TYPE::READ){
		//===std::cout << convert_to_string(std::this_thread::get_id()) + "--RSRV--Release-" + path + "-" + std::to_string(item->pos) + "\n";
		assert(node.get() == item->node.get());
		cache::remove_item(path, item->pos);
	}
	
	//cache::leaf_cache_m.unlock();
	
	// Unlock item
	auto it = item->item->second;
	if(type == tree_t::PROCESS_TYPE::WRITE){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_ITEM_WRITE-" + item->item->first + "\n";
		it->m.unlock();
	}
	else{
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_ITEM_READ-" + item->item->first + "\n";
		it->g.lock();
		if(--it->c == 0){
			it->m.unlock();
		}
		it->g.unlock();
	}
	
	//cache::leaf_cache_m.lock();
	
	assert(cache::leaf_cache_r.count(path));
	assert(cache::leaf_cache_r[path].second > 0);
	cache::leaf_cache_r[path].second--;
	cache::check_leaf_ref(path);
	
	cache::leaf_cache_m.unlock();
	
	auto& ch_node = node->data.change_locks;
	if(type == tree_t::PROCESS_TYPE::READ){
		ch_node.g.lock();
		if(--ch_node.c == 0){
			ch_node.m.unlock();
		}
		ch_node.g.unlock();
	}
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ITEM_RELEASE_END\n";
}

void forest::Tree::d_item_move(tree_t::node_ptr node, bool release, tree_t* tree)
{
	if(!node->get_childs()){
		return;
	}
	
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "ITEM_MOVE" << std::endl;
	cache::leaf_cache_m.lock();
	
	auto childs = node->get_childs();
	int childs_size = node->childs_size();
	
	std::unordered_map<int,int> new_vals;
	
	if(release){
		node = nullptr;
	}
	
	//uintptr_t nptr;
	
	string npath;
	
	if(node && has_data(node)){
		npath = get_node_data(node)->path;
		node = get_leaf(npath);	
	}
	
	//if(node){
	//	nptr = reinterpret_cast<uintptr_t>(node.get());
	//}
	
	for(int i=0;i<childs_size;i++){
		tree_t::child_item_type_ptr it = (*childs)[i];
		if(!it->node || !node){
			continue;
		}
		node_ptr n = it->node;
		string path = get_node_data(n)->path;
		//if(has_data(n)){
		//	n = get_leaf(get_node_data(n)->path);
		//}
		//uintptr_t ptr = reinterpret_cast<uintptr_t>(n.get());
		if(!cache::leaf_cache_i.count(path) || !cache::leaf_cache_i[path].count(it->pos)){
			continue;
		}
		
		assert(has_data(n));
		int cnt = cache::leaf_cache_i[path][it->pos];
		new_vals[i] = cnt;
		
		if(node.get() != n.get()){
			string path = get_node_data(n)->path;
			
			assert(cache::leaf_cache_r.count(path));
			assert(cache::leaf_cache_r[path].second >= cnt);
			assert(cache::leaf_cache_r.count(npath));
			
			//===std::cout << convert_to_string(std::this_thread::get_id()) + "--RSRV--Move-" + path + "-" + npath + "-" + to_string(cnt) + "\n";
			
			cache::leaf_cache_r[path].second -= cnt;
			cache::leaf_cache_r[npath].second += cnt;
			
			cache::leaf_cache_i[path].erase(it->pos);
			if(!cache::leaf_cache_i[path].size()){
				cache::leaf_cache_i.erase(path);
			}
			
			cache::check_leaf_ref(path);
		}
	}
	
	for(int i=0;i<childs_size;i++){
		tree_t::child_item_type_ptr it = (*childs)[i];
		it->item->second->o.lock();
		it->node = node;
		it->pos = i;
		it->item->second->o.unlock();
	}
	if(new_vals.size()){
		cache::leaf_cache_i[npath] = new_vals;
	}
	
	cache::leaf_cache_m.unlock();
}

void forest::Tree::d_leaf_insert(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree)
{
	assert(has_data(node));
	
	cache::leaf_cache_m.lock();
	node = get_leaf(get_node_data(node)->path);
	
	if(item){
		string path = get_node_data(node)->path;
		assert(cache::leaf_cache_r.count(path));
		assert(cache::leaf_cache_r[path].second > 0);
		cache::leaf_cache_r[path].second++;
		
		/*if(item && !item->node){
			
		} 
		else {
			cache::insert_item(node, item->pos);
		} */
	}
	
	cache::leaf_cache_m.unlock();
	
	// Prepare flag
	//std::shared_ptr<bool> b = std::shared_ptr<bool>(new bool(true));
	
	// Quick-access
	auto& ch_node = node->data.change_locks;
	
	// Assign flag
	//ch_node.shared_lock = b;
	
	ch_node.p.lock();
	ch_node.shared_lock = true;
	if(item){
		item->item->second->shared_lock = true;
	}
	ch_node.p.unlock();
	
	// Lock
	if(item){
		std::lock(item->item->second->m, ch_node.m);
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_ITEM_WRITE-" + item->item->first + "\n";
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(node)->path + "\n";
		
	}
	else{
		ch_node.m.lock();	
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(node)->path + "\n";
	}
	
	ch_node.p.lock();
	ch_node.shared_lock = false;
	if(item){
		item->item->second->shared_lock = false;
	}
	ch_node.cond.notify_all();
	ch_node.p.unlock();
	
	// Cleanup
	//ch_node.shared_lock = nullptr;
	
	// Notify threads
	//ch_node.cond.notify_all();
}

void forest::Tree::d_leaf_delete(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "delete_start\n";
	assert(has_data(node));
	
	// Get orig node, remember reserved item
	cache::leaf_cache_m.lock();
	node = item->node;
	if(has_data(node)){
		node = get_leaf(get_node_data(node)->path);
	}
	
	string path = get_node_data(node)->path;
	assert(cache::leaf_cache_r.count(path));
	assert(cache::leaf_cache_r[path].second > 0);
	cache::leaf_cache_r[path].second++;
	
	//cache::insert_item(node, item->pos);
	cache::leaf_cache_m.unlock();
	
	// Prepare flag
	//std::shared_ptr<bool> b = std::shared_ptr<bool>(new bool(true));
	
	// Quick-access
	auto& ch_node = node->data.change_locks;
	
	// Assign flag
	//ch_node.shared_lock = b;
	
	ch_node.p.lock();
	ch_node.shared_lock = true;
	item->item->second->shared_lock = true;
	ch_node.p.unlock();
	
	// Lock
	std::lock(item->item->second->m, ch_node.m);
	
	ch_node.p.lock();
	ch_node.shared_lock = false;
	item->item->second->shared_lock = false;
	ch_node.cond.notify_all();
	ch_node.p.unlock();
	
	item->item->second->set_file(nullptr);
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_ITEM_WRITE-" + item->item->first + "\n";
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + path + "\n";
	
	// Cleanup
	//ch_node.shared_lock = nullptr;
	
	// Notify threads
	//ch_node.cond.notify_all();
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "delete_ends\n";
}

void forest::Tree::d_leaf_split(tree_t::node_ptr node, tree_t::node_ptr new_node, tree_t::node_ptr link_node, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_start\n";
	if(!link_node){
		//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_ends\n";
		d_leaf_shift(node, new_node, tree);
		return;
	}
	
	// Make sure all is going as it should
	assert(has_data(node));
	assert(has_data(new_node));
	assert(has_data(link_node));
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_start_get_before_gets\n";
	
	// Get the original nodes
	cache::leaf_cache_m.lock();
	node = get_leaf(get_node_data(node)->path);
	new_node = get_leaf(get_node_data(new_node)->path);
	link_node = get_leaf(get_node_data(link_node)->path);
	cache::leaf_cache_m.unlock();
	
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_start_get_leafs\n";
	
	// quick-access
	auto& ch_node = node->data.change_locks;
	auto& ch_new_node = new_node->data.change_locks;
	auto& ch_link_node = link_node->data.change_locks; 
	
	// Priority lock
	ch_node.p.lock();
	ch_new_node.p.lock();
	ch_link_node.p.lock();
	
	// assing flag
	ch_node.shared_lock = true;
	ch_new_node.shared_lock = true;
	ch_link_node.shared_lock = true;
	
	// Priority unlock
	ch_node.p.unlock();
	ch_new_node.p.unlock();
	ch_link_node.p.unlock();
	
	// Lock nodes
	std::lock(ch_node.m, ch_new_node.m, ch_link_node.m);
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(node)->path + "\n";
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(new_node)->path + "\n";
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(link_node)->path + "\n";
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_start_lock_nodes\n";
	
	// Priority lock
	ch_node.p.lock();
	ch_new_node.p.lock();
	ch_link_node.p.lock();
	
	// Cleanup
	ch_node.shared_lock = false;
	ch_new_node.shared_lock = false;
	ch_link_node.shared_lock = false;
	
	// Notify threads
	ch_node.cond.notify_all();
	ch_new_node.cond.notify_all();
	ch_link_node.cond.notify_all();
	
	// Priority unlock
	ch_node.p.unlock();
	ch_new_node.p.unlock();
	ch_link_node.p.unlock();
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "join_ends\n";
}

void forest::Tree::d_leaf_join(tree_t::node_ptr node, tree_t::node_ptr join_node, tree_t::node_ptr link_node, tree_t* tree)
{
	d_leaf_split(node, join_node, link_node, tree);
}

void forest::Tree::d_leaf_shift(tree_t::node_ptr node, tree_t::node_ptr shift_node, tree_t* tree)
{
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "shift_start\n";
	// Make sure all is going as it should
	assert(has_data(node));
	assert(has_data(shift_node));
	
	// Get original nodes
	cache::leaf_cache_m.lock();
	node = get_leaf(get_node_data(node)->path);
	shift_node = get_leaf(get_node_data(shift_node)->path);
	cache::leaf_cache_m.unlock();
	
	// quick-access
	auto& ch_node = node->data.change_locks;
	auto& ch_shift_node = shift_node->data.change_locks;
	
	// Priority lock
	ch_node.p.lock();
	ch_shift_node.p.lock();
	
	// Assign flag
	ch_node.shared_lock = true;
	ch_shift_node.shared_lock = true;
	
	// Priority unlock
	ch_node.p.unlock();
	ch_shift_node.p.unlock();
	
	// Lock
	std::lock(ch_node.m, ch_shift_node.m);
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(node)->path + "\n";
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-LOCK_CHANGE_WRITE-" + get_node_data(shift_node)->path + "\n";
	
	// Priority lock
	ch_node.p.lock();
	ch_shift_node.p.lock();
	
	// Cleanup
	ch_node.shared_lock = false;
	ch_shift_node.shared_lock = false;
	
	// Notify threads
	ch_node.cond.notify_all();
	ch_shift_node.cond.notify_all();
	
	// Priority unlock
	ch_node.p.unlock();
	ch_shift_node.p.unlock();
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "shift_ends\n";
}

void forest::Tree::d_leaf_free(tree_t::node_ptr node, tree_t* tree)
{
	if(!node){
		return;
	}
	
	assert(has_data(node));
	
	cache::leaf_cache_m.lock();
	node = get_leaf(get_node_data(node)->path);
	cache::leaf_cache_m.unlock();
	
	//LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "-UNLOCK_CHANGE_WRITE-" + get_node_data(node)->path + "\n";
	node->data.change_locks.m.unlock();
}

void forest::Tree::d_leaf_ref(tree_t::node_ptr node, tree_t::node_ptr ref_node, tree_t::LEAF_REF ref, tree_t* tree)
{
	/*if(ref == tree_t::LEAF_REF::NEXT){
		node->set_next_leaf(ref_node);
	} else {
		node->set_prev_leaf(ref_node);
	}
	return;*/
	//assert(has_data(node));
	string ref_path = "-";
	if(ref_node){
		assert(has_data(ref_node));
		ref_path = get_node_data(ref_node)->path;
	}
	node_data_ptr data; 
	node_ptr n;
	if(has_data(node)){
		cache::leaf_cache_m.lock();
		string cur_path = get_node_data(node)->path;
		if(cache::leaf_cache_r.count(cur_path)){
			n = cache::leaf_cache_r[cur_path].first;
			data = get_node_data(n);
		}
		//n = get_leaf(get_node_data(node)->path);
		cache::leaf_cache_m.unlock();
		//data = get_node_data(n);
	}
	if(ref == tree_t::LEAF_REF::NEXT){
		node->set_next_leaf(ref_node);
		if(data){
			data->next = ref_path;
		}
	} else {
		node->set_prev_leaf(ref_node);
		if(data){
			data->prev = ref_path;
		}
	}
}

void forest::Tree::d_save_base(tree_t::node_ptr node, tree_t* tree)
{
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "SAVE_BASE_START" << std::endl;
	
	// Save Base File
	string base_file_name = this->get_name();
	DBFS::File* base_f = DBFS::create();
	tree_base_read_t base_d;
	base_d.type = this->get_type();
	base_d.count = tree->size();
	base_d.factor = tree->get_factor();
	
	tree_t::node_ptr root_node = tree->get_root_pub();
	base_d.branch_type = (root_node->is_leaf() ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	node_data_ptr base_data = get_node_data(root_node);
	base_d.branch = base_data->path;
	write_base(base_f, base_d);
	string new_base_file_name = base_f->name();
	
	base_f->close();
	DBFS::remove(base_file_name);
	DBFS::move(new_base_file_name, base_file_name);
	
	////LOGS// std::cout << convert_to_string(std::this_thread::get_id()) + "SAVE_BASE_END" << std::endl;
}

