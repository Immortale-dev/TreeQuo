#include "tree.hpp"

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
	tree_base_read_t ret;
	DBFS::File* f = new DBFS::File(filename);
	
	int t;
	int lt;
	
	f->read(ret.count);
	f->read(ret.factor);
	f->read(t); ret.type = (TREE_TYPES)t;
	f->read(ret.branch);
	f->read(lt); ret.branch_type = NODE_TYPES(lt);

	f->close();
	delete f;
	return ret;
}

forest::Tree::tree_intr_read_t forest::Tree::read_intr(string filename)
{
	using key_type = tree_t::key_type;
	
	int t, c;
	
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
	
	own_lock(node);
	
	if(!has_data(node)){
		// Define data for node
		string temp_path = DBFS::random_filename();
		
		cache::intr_lock();
		/// lock{
		n = create_node(temp_path, NODE_TYPES::INTR);
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
	if(node->data.travel_locks.wlock){
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
		n = create_node(temp_path, NODE_TYPES::LEAF);
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
	if(node->data.travel_locks.wlock){
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
	
	next_leaf = create_node(ndata->next, NODE_TYPES::LEAF);
	prev_leaf = create_node(ndata->prev, NODE_TYPES::LEAF);
	
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
	// Get node data
	node_data_ptr data = get_node_data(node);
	string path = data->path;
	
	// Unlock original node if it was not already deleted
	cache::leaf_lock();
	/// lock{
	tree_t::node_ptr n = get_original(node);
	if(is_write_locked(node)){
		unlock_write(n);
	}
	else{
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
		data->ghost = true;
	}
	/// }own_lock
	own_unlock(node);
	
	cache::release_node(n, true);
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
			n = node_ptr(new typename tree_t::InternalNode());
		} else {
			n = node_ptr(new typename tree_t::LeafNode());
		}
		set_node_data(n, create_node_data(true, child_path));
		intr_data->add_nodes(i,n);
	}
	set_node_data(intr_data, create_node_data(false, path));
	
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
	change_lock_write(leaf_data);
	
	// Put it into the cache
	cache::leaf_cache_r[path] = std::make_pair(leaf_data,1);
	cache::leaf_unlock();
	
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
	assert(has_data(node));
	
	string& path = get_node_data(node)->path;
	bool is_leaf = node->is_leaf();
	
	if(is_leaf){
		return get_leaf(path);
	} else {
		return get_intr(path);
	}
}

forest::tree_t::node_ptr forest::Tree::extract_node(tree_t::child_item_type_ptr item)
{
	std::lock_guard<std::mutex> lock(item->item->second->o);
	return item->node;
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
		node = get_leaf(path);
		/// }lock
		cache::leaf_unlock();
		
		// Check for priority
		if(w_prior){
			auto& ch_node = node->data.change_locks;
			std::unique_lock<std::mutex> plock(ch_node.p);
			while(ch_node.shared_lock && !item->item->second->shared_lock){
				ch_node.cond.wait(plock);
			}
		}
		
		change_lock_read(node);
		
		// If it is still the same node - break the loop
		if(path == get_node_data(item->node)->path){
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
		// TODO: throw error here
	}
	
	delete[] buf;
	data->set_start(start_data);
	data->set_file(file);
	data->delete_cache();
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

void forest::Tree::d_leave(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
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

void forest::Tree::d_insert(tree_t::node_ptr node, tree_t* tree)
{	
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
		
		// Block any interactions with file until it is ready
		auto lock = fp->get_lock();
		
		for(auto& it : *(node->get_childs()) ){
			write_leaf_item(fp, it->item->second);
		}
		
		fp->stream().flush();
		
		node_data_ptr data = get_node_data(node);
		string cur_name = data->path;
		
		DBFS::remove(cur_name);
		fp->move(cur_name);
	}
}

void forest::Tree::d_remove(tree_t::node_ptr node, tree_t* tree)
{	
	if(!has_data(node)){
		assert(false);
		return;
	}
	
	node_data_ptr data = get_node_data(node);
	
	// TODO: RECODE WHEN DELAYED SAVE IMPLEMENTED
	if(node->is_leaf() && node->size()){
		node->first_child()->item->second->file->close();
		node->get_childs()->resize(0);
	}
	
	DBFS::remove(data->path);
	cache::clear_node_cache(node);
}

void forest::Tree::d_reserve(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{	
	string& path = get_node_data(node)->path;
	
	cache::leaf_lock();
	/// lock{
	node = get_leaf(path);
	cache::reserve_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	change_lock_type(node, type);
}

void forest::Tree::d_release(tree_t::node_ptr node, tree_t::PROCESS_TYPE type, tree_t* tree)
{	
	string& path = get_node_data(node)->path;
	
	cache::leaf_lock();
	/// lock{
	node = get_leaf(path);
	cache::release_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	change_unlock_type(node, type);
}

void forest::Tree::d_before_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{
	if(!item){
		return;
	}
	
	tree_t::node_ptr node = extract_locked_node(item, true);
	cache::reserve_node(node, true);
	
	if( (step < 0 && item->pos == 0) || (step > 0 && item->pos+1 == node->childs_size()) ){
		
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

void forest::Tree::d_after_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{	
	if(!item || !step){
		return;
	}
	
	cache::leaf_lock();
	/// lock{
	tree_t::node_ptr node = extract_node(item);
	tree_t::node_ptr node_old;

	string path = get_node_data(node)->path;
	string path_old;
	
	node = get_leaf(path);
	
	if( (step > 0 && item->pos == 0) || (step < 0 && item->pos+1 == node->childs_size()) ){
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

void forest::Tree::d_item_reserve(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree)
{	
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_lock();
		/// lock{
		node = extract_node(item);
		path = get_node_data(node)->path;
		node = get_leaf(path);
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
		cache::insert_item(path, item->pos);
	}
	/// }lock
	cache::leaf_unlock();
	
	// Lock item
	lock_type(item, type);
	
	// Change unlock if it is READ reserve as it was locked in `extract_locked_node`
	if(type == tree_t::PROCESS_TYPE::READ){
		change_unlock_read(node);
	}
}

void forest::Tree::d_item_release(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type, tree_t* tree)
{
	tree_t::node_ptr node;
	string path;
	
	if(type == tree_t::PROCESS_TYPE::WRITE){
		cache::leaf_lock();
		node = item->node;
		path = get_node_data(node)->path;
		node = get_leaf(path);
		cache::leaf_unlock();
	}
	else{
		node = extract_locked_node(item);
		path = get_node_data(node)->path;
	}
	
	cache::leaf_lock();
	/// lock{
	if(type == tree_t::PROCESS_TYPE::READ){
		cache::remove_item(path, item->pos);
	}
	// Unlock item
	unlock_type(item, type);
	cache::release_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	// Change unlock if it is READ release as it was locked in `extract_locked_node`
	if(type == tree_t::PROCESS_TYPE::READ){
		change_unlock_read(node);
	}
}

void forest::Tree::d_item_move(tree_t::node_ptr node, bool release, tree_t* tree)
{
	if(!node->get_childs()){
		return;
	}
	
	cache::leaf_lock();
	
	auto childs = node->get_childs();
	int childs_size = node->childs_size();
	
	std::unordered_map<int,int> new_vals;
	
	if(release){
		node = nullptr;
	}
	
	string npath;
	
	if(node && has_data(node)){
		npath = get_node_data(node)->path;
		node = get_leaf(npath);	
	}
	
	for(int i=0;i<childs_size;i++){
		tree_t::child_item_type_ptr it = (*childs)[i];
		if(!it->node || !node){
			continue;
		}
		node_ptr n = it->node;
		string path = get_node_data(n)->path;
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
	
	cache::leaf_unlock();
}

void forest::Tree::d_leaf_insert(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree)
{	
	cache::leaf_cache_m.lock();
	/// lock{
	string path = get_node_data(node)->path;
	node = get_leaf(path);
	cache::leaf_cache_r[path].second++;
	/// }lock
	cache::leaf_cache_m.unlock();
	
	// Lock both at once
	change_lock_bunch(node, item, true);
}

void forest::Tree::d_leaf_delete(tree_t::node_ptr node, tree_t::child_item_type_ptr item, tree_t* tree)
{
	cache::leaf_lock();
	/// lock{
	string path = get_node_data(node)->path;
	node = get_leaf(path);
	cache::reserve_leaf_node(path);
	/// }lock
	cache::leaf_unlock();
	
	// Lock both at once
	change_lock_bunch(node, item);
	
	item->item->second->set_file(nullptr);
}

void forest::Tree::d_leaf_split(tree_t::node_ptr node, tree_t::node_ptr new_node, tree_t::node_ptr link_node, tree_t* tree)
{
	if(!link_node){
		d_leaf_shift(node, new_node, tree);
		return;
	}
	
	// Get the original nodes
	cache::leaf_lock();
	/// lock{
	node = get_leaf(get_node_data(node)->path);
	new_node = get_leaf(get_node_data(new_node)->path);
	link_node = get_leaf(get_node_data(link_node)->path);
	/// }lock
	cache::leaf_unlock();
	
	// Lock all at once
	change_lock_bunch(node, new_node, link_node, true);
}

void forest::Tree::d_leaf_join(tree_t::node_ptr node, tree_t::node_ptr join_node, tree_t::node_ptr link_node, tree_t* tree)
{
	d_leaf_split(node, join_node, link_node, tree);
}

void forest::Tree::d_leaf_shift(tree_t::node_ptr node, tree_t::node_ptr shift_node, tree_t* tree)
{
	// Get original nodes
	cache::leaf_lock();
	/// lock{
	node = get_leaf(get_node_data(node)->path);
	shift_node = get_leaf(get_node_data(shift_node)->path);
	/// }lock
	cache::leaf_unlock();
	
	// Lock all at once
	change_lock_bunch(node, shift_node, true);
}

void forest::Tree::d_leaf_free(tree_t::node_ptr node, tree_t* tree)
{
	if(!node){
		return;
	}
	
	assert(has_data(node));
	
	cache::leaf_lock();
	/// lock{
	node = get_leaf(get_node_data(node)->path);
	/// }lock
	cache::leaf_unlock();
	node->data.change_locks.m.unlock();
}

void forest::Tree::d_leaf_ref(tree_t::node_ptr node, tree_t::node_ptr ref_node, tree_t::LEAF_REF ref, tree_t* tree)
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

void forest::Tree::d_save_base(tree_t::node_ptr node, tree_t* tree)
{
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
}
