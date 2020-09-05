#include "tree.hpp"

forest::Tree::Tree(string path)
{	
	name = path;
	tree_base_read_t base = read_base(path);
	
	type = base.type;
	
	// Init BPT
	tree = new tree_t(base.factor, create_node(base.branch, base.branch_type), base.count, this);
}

forest::Tree::Tree()
{
	tree = nullptr;
	// ctor
}

forest::Tree::Tree(string path, TREE_TYPES type, int factor)
{
	name = path;
	this->type = type;
	
	// Init BPT
	tree = new tree_t(factor, create_node(LEAF_NULL, NODE_TYPES::LEAF), 0, this);
}

forest::Tree::~Tree()
{
	if(tree){
		delete tree;
		tree = nullptr;
	}
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
	
	std::unique_lock<std::mutex> tree_lock(cache::tree_cv_m);
	
	while(!cache::tree_cache_r.count(path) && cache::tree_cache_q.count(path)){
		cache::tree_cv.wait(tree_lock);
	}
	
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
	
	cache::tree_cache_q.insert(path);
	
	// Unlock global mutexes
	tree_lock.unlock();
	cache::tree_cache_m.unlock();
	
	// Get from file
	t = tree_ptr(new Tree(path));
	
	// Fill in cache and send future event
	cache::tree_cache_m.lock();
	tree_lock.lock();
	
	cache::tree_cache.push(path, t);
	cache::tree_cache_r[path] = std::make_pair(t,0);
	cache::tree_cache_q.erase(path);
	
	// Notify all threads
	cache::tree_cv.notify_all();
	
	// return value
	return t;
}

forest::tree_t* forest::Tree::get_tree()
{
	return this->tree;
}

void forest::Tree::set_tree(tree_t* tree)
{
	this->tree = tree;
}

void forest::Tree::insert(tree_t::key_type key, tree_t::val_type val, bool update)
{
	tree->insert(make_pair(key, std::move(val)), update);
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
		log_error("[Tree::seed_tree] (cannot create file)");
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_FILE);
		return;
	}
	f->write("0 " + to_string(factor) + " " + to_string((int)type) + " " + LEAF_NULL + " " + to_string((int)NODE_TYPES::LEAF) + "\n");
	f->close();
	delete f;
}

void forest::Tree::tree_reserve()
{
	if(name == ROOT_TREE){
		return;
	}
	cache::tree_cache_m.lock();
	assert(cache::tree_cache_r.count(name));
	assert(cache::tree_cache_r[name].second > 0);
	cache::tree_cache_r[name].second++;
	cache::tree_cache_m.unlock();
}

void forest::Tree::tree_release()
{
	if(name == ROOT_TREE){
		return;
	}
	cache::tree_cache_m.lock();
	assert(cache::tree_cache_r.count(name));
	assert(cache::tree_cache_r[name].second > 0);
	cache::tree_cache_r[name].second--;
	cache::check_tree_ref(name);
	cache::tree_cache_m.unlock();
}

forest::tree_base_read_t forest::Tree::read_base(string filename)
{	
	// Wait for file to become ready
	savior->get(filename);
	
	tree_base_read_t ret;
	DBFS::File* f = new DBFS::File(filename);
	
	int t;
	int lt;
	
	f->seekg(0);
	f->read(ret.count);
	f->read(ret.factor);
	f->read(t); ret.type = (TREE_TYPES)t;
	f->read(ret.branch);
	f->read(lt); ret.branch_type = NODE_TYPES(lt);
	
	if(f->fail()){
		log_error("[Tree::read_base] (cannot read file)");
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_READ_FILE);
	}

	f->close();
	delete f;
	
	return ret;
}

forest::tree_intr_read_t forest::Tree::read_intr(string filename)
{	
	// Wait for file to become ready
	savior->get(filename);
	using key_type = tree_t::key_type;
	
	int t, c;
	
	DBFS::File* f = new DBFS::File(filename);
	
	f->seekg(0);
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
	
	if(f->fail()){
		log_error("[Tree::read_intr] (cannot read file)");
		delete keys;
		delete vals;
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_READ_FILE);
	}
	
	f->close();
	delete f;
	
	tree_intr_read_t d;
	d.childs_type = (NODE_TYPES)t;
	d.child_keys = keys;
	d.child_values = vals;
	
	return d;
}

forest::tree_leaf_read_t forest::Tree::read_leaf(string filename)
{	
	// Wait for file to be ready
	savior->get(filename);
	
	DBFS::File* f = create_leaf_file(filename, true);
	
	int c;
	string left_leaf, right_leaf;
	uint_t start_data;
	
	f->seekg(0);
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
	start_data = f->tellg()+1;
	
	if(f->fail()){
		log_error("[Tree::read_leaf] (cannot read file)");
		delete keys;
		delete vals_lengths;
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_READ_FILE);
	}
	
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
	
	own_lock(node);
	
	if(!has_data(node)){
		// Define data for node
		string temp_path = DBFS::random_filename();
		
		cache::intr_lock();
		/// lock{
		n = tree_t::node_ptr(new tree_t::InternalNode(node->get_keys(), node->get_nodes()));
		set_node_data(n, create_node_data(true, temp_path));
		data = create_node_data(false, temp_path);
		set_node_data(node, data);
		cache::intr_insert(n);
		cache::reserve_intr_node(temp_path);
		/// }lock
		cache::intr_unlock();
		
		own_unlock(node);
	} else {
		// Work with the node
		own_unlock(node);
		
		data = get_node_data(node);
		
		cache::intr_lock();
		/// lock{
		n = get_original(node);
		cache::reserve_intr_node(data->path);
		/// }lock
		cache::intr_unlock();
	}
	
	// Lock the original node
	if(get_data(node).travel_locks.wlock){
		lock_write(n);
	} else {
		lock_read(n);
	}
	
	// Owners lock made for readers-writer concept
	own_lock(node);
	/// own_lock{
	if(!own_inc(node)){
		node->set_keys(n->get_keys());
		node->set_nodes(n->get_nodes());
		data->ghost = false;
	}
	/// }own_lock
	own_unlock(node);
}

void forest::Tree::materialize_leaf(tree_t::node_ptr node)
{
	tree_t::node_ptr n, next_leaf, prev_leaf;
	node_data_ptr data;
	
	own_lock(node);
	
	if(!has_data(node)){
		// Define data for node
		string temp_path = DBFS::random_filename();
		
		cache::leaf_lock();
		/// lock{
		n = tree_t::node_ptr(new tree_t::LeafNode(node->get_childs()));
		set_node_data(n, create_node_data(true, temp_path));
		data = create_node_data(false, temp_path);
		set_node_data(node, data);
		cache::leaf_insert(n);
		cache::reserve_leaf_node(temp_path);
		/// }lock
		cache::leaf_unlock();
		
		own_unlock(node);
	} else {
		// Work with node
		own_unlock(node);
	
		data = get_node_data(node);
		
		cache::leaf_lock();
		/// lock{
		n = get_original(node);
		cache::reserve_leaf_node(data->path);
		/// }lock
		cache::leaf_unlock();
	}
	
	// Lock the original node
	if(get_data(node).travel_locks.wlock){
		lock_write(n);
	} else {
		lock_read(n);
	}
	
	// Own lock made for readers-writer concept
	own_lock(node);
	/// own_lock{
	if(own_inc(node)){
		own_unlock(node);
		return;
	}
	
	node->set_childs(n->get_childs());
	node_data_ptr ndata = get_node_data(n);
	
	next_leaf = create_node(ndata->next, NODE_TYPES::LEAF, true);
	prev_leaf = create_node(ndata->prev, NODE_TYPES::LEAF, true);
	
	if(ndata->prev != LEAF_NULL){
		node->set_prev_leaf(prev_leaf);
	}
	if(ndata->next != LEAF_NULL){
		node->set_next_leaf(next_leaf);
	}
	data->ghost = false;
	/// }own_lock
	own_unlock(node);
}

void forest::Tree::unmaterialize_intr(tree_t::node_ptr node)
{
	// Get node data
	node_data_ptr data = get_node_data(node);
	
	// Unlock original node if it was not already deleted
	cache::intr_lock();
	/// lock{
	tree_t::node_ptr n = get_original(node);
	if(is_write_locked(node)){
		unlock_write(n);
	} else {
		unlock_read(n);
	}
	/// }lock
	cache::intr_unlock();
	
	// made for readers-writer concept
	own_lock(node);
	/// own_lock{
	if(!own_dec(node)){
		node->set_keys(nullptr);
		node->set_nodes(nullptr);
		data->ghost = true;
	}
	/// }own_lock
	own_unlock(node);
	
	cache::release_node(n, true);
}

void forest::Tree::unmaterialize_leaf(tree_t::node_ptr node)
{
	// Unlock original node if it was not already deleted
	cache::leaf_lock();
	/// lock{
	tree_t::node_ptr n = get_original(node);
	if(is_write_locked(node)){
		unlock_write(n);
	} else {
		unlock_read(n);
	}
	/// }lock
	cache::leaf_unlock();
	
	// Owner lock made for readers-writer concept
	own_lock(node);
	/// own_lock{
	if(!own_dec(node)){
		node->set_childs(nullptr);
		node->set_prev_leaf(nullptr);
		node->set_next_leaf(nullptr);
		get_node_data(node)->ghost = true;
	}
	/// }own_lock
	own_unlock(node);
	
	cache::release_node(n, true);
}

DBFS::File* forest::Tree::create_leaf_file(string filename, bool lock_for_limit)
{
	return new DBFS::File(filename);
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

forest::node_ptr forest::Tree::create_node(string path, NODE_TYPES node_type, bool empty)
{
	tree_t::Node* node;
	if(node_type == NODE_TYPES::INTR){
		node = new tree_t::InternalNode(nullptr, nullptr);
	} else {
		node = new tree_t::LeafNode(nullptr);
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

void forest::Tree::set_type(TREE_TYPES type)
{
	this->type = type;
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
	get_data(intr_data).is_original = true;
	lock_write(intr_data);
	
	// Put it into the cache
	cache::intr_cache_r[path] = std::make_pair(intr_data,1);
	cache::intr_unlock();
	
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
			n = node_ptr(new typename tree_t::InternalNode(nullptr, nullptr));
		} else {
			n = node_ptr(new typename tree_t::LeafNode(nullptr));
		}
		set_node_data(n, create_node_data(true, child_path));
		intr_data->add_nodes(i,n);
	}
	set_node_data(intr_data, create_node_data(false, path));
	
	// Clear memory
	delete keys_ptr;
	delete vals_ptr;
	
	// Unlock node
	cache::intr_lock();
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
	get_data(leaf_data).is_original = true;
	lock_write(leaf_data);
	change_lock_write(leaf_data);
	
	// Put it into the cache
	cache::leaf_cache_r[path] = {leaf_data,1};
	cache::leaf_unlock();
	
	// Fill data
	tree_leaf_read_t leaf_d = read_leaf(path);
	std::vector<tree_t::key_type>* keys_ptr = leaf_d.child_keys;
	std::vector<uint_t>* vals_length = leaf_d.child_lengths;
	uint_t start_data = leaf_d.start_data;
	std::shared_ptr<DBFS::File> f(leaf_d.file);
	get_data(leaf_data).f = f;
	int c = keys_ptr->size();
	uint_t last_len = 0;
	
	for(int i=0;i<c;i++){
		leaf_data->insert(this->tree->create_entry_item( (*keys_ptr)[i], file_data_ptr(new file_data_t(f, start_data+last_len, (*vals_length)[i])) ));
		last_len += (*vals_length)[i];
	}
	
	// Clear memory
	delete keys_ptr;
	delete vals_length;
	
	// Update records positions
	///int childs_size = leaf_data->childs_size();
	auto childs = leaf_data->get_childs();
	
	// Update node for RBTree
	tree_t::childs_type_iterator start = childs->begin();
	while(start != childs->end()){
		start->data->node = leaf_data;
		start = childs->find_next(start);
	}
	
	set_node_data(leaf_data, create_node_data(false, path, leaf_d.left_leaf, leaf_d.right_leaf));
	
	// Unlock node and push to cache
	cache::leaf_lock();
	if(!cache::leaf_cache.has(path)){
		cache::leaf_cache.push(path, leaf_data);
	}
	assert(cache::leaf_cache_r[path].second > 0);
	cache::leaf_cache_r[path].second--;
	change_unlock_write(leaf_data);
	unlock_write(leaf_data);
	
	// Return
	return leaf_data;
}

forest::tree_t::node_ptr forest::Tree::get_original(tree_t::node_ptr node)
{
	node_ptr n;
	
	if(get_data(node).is_original){
		return node;
	}

	auto orig = get_data(node).original.lock();
	if(orig){
		n = std::static_pointer_cast<tree_t::Node>(orig);
		if(get_data(n).bloomed){
			string& path = get_node_data(node)->path;
			if(node->is_leaf()){
				if(cache::leaf_cache.has(path)){
					cache::leaf_cache.get(path);
				} else {
					cache::leaf_cache.push(path, n);
				}
			} else {
				if(cache::intr_cache.has(path)){
					cache::intr_cache.get(path);
				} else {
					cache::intr_cache.push(path, n);
				}
			}
			return n;
		}
	}
	
	string& path = get_node_data(node)->path;
	
	
	if(node->is_leaf()){
		n = get_leaf(path);
	} else {
		n = get_intr(path);
	}
	get_data(node).original = n;
	return n;
}

forest::tree_t::node_ptr forest::Tree::extract_node(tree_t::child_item_type_ptr item)
{
	std::lock_guard<std::mutex> lock(item->item->second->o);
	return item->node.lock();
}

forest::tree_t::node_ptr forest::Tree::extract_locked_node(tree_t::child_item_type_ptr item, bool w_prior)
{	
	tree_t::node_ptr node;
	// Make sure to lock the right node
	do{	
		
		cache::leaf_lock();
		/// lock{
		node = extract_node(item);
		string path = get_node_data(node)->path;
		node = get_original(node);
		/// }lock
		cache::leaf_unlock();
		
		// Check for priority
		if(w_prior){
			auto& ch_node = get_data(node).change_locks;
			std::unique_lock<std::mutex> plock(ch_node.p);
			while(ch_node.shared_lock && !item->item->second->shared_lock){
				ch_node.cond.wait(plock);
			}
		}
		
		change_lock_read(node);
		
		// If it is still the same node - break the loop
		if(path == get_node_data(item->node.lock())->path){
			break;
		}
		
		change_unlock_read(node);
	}while(true);
	return node;
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
	
	// Clear memory
	delete keys;
	delete paths;
	
	if(file->fail()){
		log_error("[Tree::write_intr] (cannot write file)");
		throw DBException(DBException::ERRORS::CANNOT_WRITE_FILE);
	}
}

void forest::Tree::write_base(DBFS::File* file, tree_base_read_t data)
{
	file->write( to_string(data.count) + " " + to_string(data.factor) + " " + to_string((int)data.type) + " " + data.branch + " " + to_string((int)data.branch_type) + "\n" );
	if(file->fail()){
		log_error("[Tree::write_base] (cannot write file)");
		throw DBException(DBException::ERRORS::CANNOT_WRITE_FILE);
	}
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
	
	// Clear memory
	delete keys;
	delete lengths;
	
	if(file->fail()){
		log_error("[Tree::write_leaf] (cannot write file)");
		throw DBException(DBException::ERRORS::CANNOT_WRITE_FILE);
	}
}

void forest::Tree::write_leaf_item(std::shared_ptr<DBFS::File> file, tree_t::val_type& data)
{
	int_t start_data = file->tellp();
	
	int read_size = CHUNK_SIZE;
	char* buf = new char[read_size];
	int rsz;
	auto reader = data->get_reader();
	
	try{
		while( (rsz = reader.read(buf, read_size)) ){
			file->write(buf, rsz);
		}
	} catch(...){
		log_error("[Tree::write_leaf_item] (cannot write file)");
		throw DBException(DBException::ERRORS::CANNOT_WRITE_FILE);
	}
	
	delete[] buf;
	data->set_start(start_data);
	data->set_file(file);
}

// Proceed
void forest::Tree::d_enter(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{	
	// Lock node mutex
	lock_type(node, type);
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		return;
	}
		
	if(!node->is_leaf()){
		materialize_intr(node);
	} else {
		materialize_leaf(node);
	}
}

void forest::Tree::d_leave(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		unlock_type(node, type);
		return;
	}
	
	if(!node->is_leaf()){
		unmaterialize_intr(node);
	} else {
		unmaterialize_leaf(node);
	}
	unlock_type(node, type);
}

void forest::Tree::d_insert(tree_t::node_ptr& node)
{	
	if(tree->is_stem_pub(node)){
		log_error("[Tree::d_insert] (stem is not expected)");
		assert(false);
	}
	
	
	if(!node->is_leaf()){
		
		cache::intr_lock();
		node_ptr n = get_original(node);
		cache::intr_unlock();
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		savior->put(cur_name, SAVE_TYPES::INTR, n);
	} else {
		
		cache::leaf_lock();
		node_ptr n = get_original(node);
		cache::leaf_unlock();
		
		assert(get_data(n).is_original);
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		savior->put(cur_name, SAVE_TYPES::LEAF, n);
	}
}

void forest::Tree::d_remove(tree_t::node_ptr& node)
{	
	if(!has_data(node)){
		log_error("[Tree::d_remove] (node must have data)");
		assert(false);
		return;
	}
	
	node_data_ptr data = get_node_data(node);
	
	node_ptr n;
	if(!node->is_leaf()){
		cache::intr_lock();
		n = get_original(node);
		cache::intr_unlock();
	} else {
		cache::leaf_lock();
		n = get_original(node);
		cache::leaf_unlock();
	}
	
	if(!node->is_leaf()){
		savior->remove(data->path, SAVE_TYPES::INTR, n);
	} else {
		n->get_childs()->clear();
		savior->remove(data->path, SAVE_TYPES::LEAF, n);
	}
	cache::clear_node_cache(node);
}

void forest::Tree::d_reserve(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{	
	string& path = get_node_data(node)->path;
	
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	cache::reserve_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	change_lock_type(node, type);
}

void forest::Tree::d_release(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{	
	string& path = get_node_data(node)->path;
	
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	cache::release_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	change_unlock_type(node, type);
}

void forest::Tree::d_before_move(tree_t::childs_type_iterator& item, int_t step)
{
	if(!item || !item->data){
		return;
	}
	
	tree_t::node_ptr node = extract_locked_node(item->data, true);
	cache::reserve_node(node, true);
	
	if( ( step < 0 && node->get_childs()->find_prev(item) == node->childs_iterator_end() ) || ( step > 0 && node->get_childs()->find_next(item) == node->childs_iterator_end() ) ){
		
		tree_t::node_ptr new_node = nullptr;
		string new_path = (step > 0) ? get_node_data(node)->next : get_node_data(node)->prev;
		
		// If no next node - break
		if(new_path != LEAF_NULL){
			
			// Reserve node (anti rc)
			cache::leaf_lock();
			/// lock{
			new_node = get_leaf(new_path);
			cache::reserve_leaf_node(new_path);
			/// }lock
			cache::leaf_unlock();
			
			change_lock_read(new_node);
		}
		
		// Reassign refs if necessary
		own_lock(node);
		/// own_lock{
		if(step > 0){
			if( ( new_node && (!node->next_leaf() || node->next_leaf().get() != new_node.get()) ) || ( !new_node && node->next_leaf() ) ){
				node->set_next_leaf(new_node);
			}
		} else {
			if( ( new_node && (!node->prev_leaf() || node->prev_leaf().get() != new_node.get()) ) || ( !new_node && node->prev_leaf() ) ){
				node->set_prev_leaf(new_node);
			}
		}
		/// }own_lock
		own_unlock(node);
	}
}

void forest::Tree::d_after_move(tree_t::childs_type_iterator& item, int_t step)
{	
	if(!item || !item->data || !step){
		return;
	}
	
	cache::leaf_lock();
	/// lock{
	tree_t::node_ptr node = extract_node(item->data);
	tree_t::node_ptr node_old;

	string& path = get_node_data(node)->path;
	string path_old;
	
	node = get_original(node);
	
	if( (step > 0 && node->get_childs()->find_prev(item) == node->childs_iterator_end()) || (step < 0 && node->get_childs()->find_next(item) == node->childs_iterator_end()) ){
		path_old = (step > 0) ? get_node_data(node)->prev : get_node_data(node)->next;
		if(path_old != LEAF_NULL){
			node_old = get_leaf(path_old);
		}
	}
	
	// Release prev node if applicable
	if(node_old){
		cache::release_leaf_node(path_old);
	}
	cache::release_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	if(node_old){
		change_unlock_read(node_old);
	}
	change_unlock_read(node);
}

void forest::Tree::d_item_reserve(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type)
{	
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_lock();
		/// lock{
		node = extract_node(item);
		path = get_node_data(node)->path;
		node = get_original(node);
		/// }lock
		cache::leaf_unlock();
	} else {
		node = extract_locked_node(item);
		path = get_node_data(node)->path;
	}
	
	cache::leaf_lock();
	/// lock{
	cache::reserve_leaf_node(path);
	if(type == tree_t::PROCESS_TYPE::READ){
		cache::insert_item(item);
	}
	/// }lock
	cache::leaf_unlock();
	
	// Reserve tree
	if(type == tree_t::PROCESS_TYPE::READ){
		tree_reserve();
	}
	
	// Lock item
	lock_type(item, type);
	
	// Change unlock if it is READ reserve as it was locked in `extract_locked_node`
	if(type == tree_t::PROCESS_TYPE::READ){
		change_unlock_read(node);
	}
}

void forest::Tree::d_item_release(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type)
{
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_lock();
		node = item->node.lock();
		path = get_node_data(node)->path;
		node = get_original(node);
		cache::leaf_unlock();
	}
	else{
		node = extract_locked_node(item);
		path = get_node_data(node)->path;
	}
	
	cache::leaf_lock();
	/// lock{
	if(type == tree_t::PROCESS_TYPE::READ){
		cache::remove_item(item);
	}
	// Unlock item
	unlock_type(item, type);
	cache::release_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	// Release tree
	if(type == tree_t::PROCESS_TYPE::READ){
		tree_release();
	}
	
	// Change unlock if it is READ release as it was locked in `extract_locked_node`
	if(type == tree_t::PROCESS_TYPE::READ){
		change_unlock_read(node);
	}
}

void forest::Tree::d_item_move(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item)
{
	if(!node->get_childs()){
		return;
	}
	
	cache::leaf_lock();
	node_ptr onode = extract_node(item);
	
	if(node && has_data(node)){
		node = get_original(node);
	}
	
	if(onode && onode.get() == node.get()){
		cache::leaf_unlock();
		return;
	}
	
	cache::leaf_cache_r[get_node_data(node)->path].second += item->item->second->res_c;
	item->node = node;
	
	if(onode){
		string& old_path = get_node_data(onode)->path;
		auto& ref = cache::leaf_cache_r[old_path];
		ref.second -= item->item->second->res_c;
		if(ref.second == 0){
			cache::check_leaf_ref(old_path);
		}
	}
	
	cache::leaf_unlock();
}

void forest::Tree::d_leaf_insert(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item)
{	
	cache::leaf_lock();
	/// lock{
	string& path = get_node_data(node)->path;
	node = get_original(node);
	cache::leaf_cache_r[path].second++;
	/// }lock
	cache::leaf_unlock();
	
	// Lock both at once
	change_lock_bunch(node, item, true);
}

void forest::Tree::d_leaf_delete(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item)
{
	cache::leaf_lock();
	/// lock{
	string path = get_node_data(node)->path;
	node = get_original(node);
	cache::reserve_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	// Lock both at once
	change_lock_bunch(node, item);
	
	item->item->second->set_file(nullptr);
}

void forest::Tree::d_leaf_split(tree_t::node_ptr& node, tree_t::node_ptr& new_node, tree_t::node_ptr& link_node)
{
	if(!link_node){
		d_leaf_shift(node, new_node);
		return;
	}
	
	// Get the original nodes
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	new_node = get_original(new_node);
	link_node = get_original(link_node);
	/// }lock
	cache::leaf_unlock();
	
	// Lock all at once
	change_lock_bunch(node, new_node, link_node, true);
}

void forest::Tree::d_leaf_join(tree_t::node_ptr& node, tree_t::node_ptr& join_node, tree_t::node_ptr& link_node)
{
	d_leaf_split(node, join_node, link_node);
}

void forest::Tree::d_leaf_shift(tree_t::node_ptr& node, tree_t::node_ptr& shift_node)
{	
	// Get original nodes
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	shift_node = get_original(shift_node);
	/// }lock
	cache::leaf_unlock();
	
	// Lock all at once
	change_lock_bunch(node, shift_node, true);
}

void forest::Tree::d_leaf_lock(tree_t::node_ptr& node)
{	
	if(!node){
		return;
	}
	
	assert(has_data(node));
	
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	/// }lock
	cache::leaf_unlock();
	get_data(node).change_locks.m.lock();
}

void forest::Tree::d_leaf_free(tree_t::node_ptr& node)
{	
	if(!node){
		return;
	}
	
	assert(has_data(node));
	
	cache::leaf_lock();
	/// lock{
	node = get_original(node);
	/// }lock
	cache::leaf_unlock();
	get_data(node).change_locks.m.unlock();
}

void forest::Tree::d_leaf_ref(tree_t::node_ptr& node, tree_t::node_ptr& ref_node, tree_t::LEAF_REF ref)
{
	string ref_path = LEAF_NULL;
	if(ref_node){
		assert(has_data(ref_node));
		ref_path = get_node_data(ref_node)->path;
	}
	node_data_ptr data; 
	node_ptr n;
	if(has_data(node)){
		cache::leaf_lock();
		/// lock{
		string cur_path = get_node_data(node)->path;
		if(cache::leaf_cache_r.count(cur_path)){
			n = cache::leaf_cache_r[cur_path].first;
			data = get_node_data(n);
		}
		/// }lock
		cache::leaf_unlock();
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

void forest::Tree::d_save_base(tree_t::node_ptr& node)
{
	// Save Base File
	string base_file_name = this->get_name();
	
	tree_ptr t;
	if(base_file_name == ROOT_TREE){
		t = FOREST;	
	} else {
		cache::tree_cache_m.lock();
		assert(cache::tree_cache_r.count(base_file_name));
		assert(cache::tree_cache_r[base_file_name].second > 0);
		t = cache::tree_cache_r[base_file_name].first;
		cache::tree_cache_m.unlock();
	}
	
	savior->put(base_file_name, SAVE_TYPES::BASE, t);
}
