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

forest::string forest::Tree::seed(TREE_TYPES type)
{
	DBFS::File* f = DBFS::create();
	string path = f->name();
	seed_tree(f, type, DEFAULT_FACTOR);
	return path;
}

forest::string forest::Tree::seed(TREE_TYPES type, string path)
{	
	DBFS::File* f = DBFS::create(path);
	string path_name = f->name();
	seed_tree(f, type, DEFAULT_FACTOR);
	return path_name;
}

forest::tree_ptr forest::Tree::get(string path)
{
	tree_ptr t;
	
	// Try to get from cache
	cache::tree_cache_m.lock();
	if(cache::tree_cache.has(path)){
		t = cache::tree_cache.get(path);
		cache::tree_cache_m.unlock();
		return t;
	}
	
	// Check reference
	if(cache::tree_cache_r.count(path)){
		t = cache::tree_cache_r[path].first;
		cache::tree_cache.push(path, t);
		cache::tree_cache_m.unlock();
		return t;
	}
	
	// Check for future
	if(cache::tree_cache_q.count(path)){
		std::shared_future<tree_ptr> f = cache::tree_cache_q[path];
		cache::tree_cache_m.unlock();
		return f.get();
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
	cache::tree_cache_m.unlock();
	
	// return value
	return t;
}

forest::tree_t* forest::Tree::get_tree()
{
	return this->tree;
}

void forest::Tree::insert(tree_t::key_type key, tree_t::val_type val)
{
	tree->insert(make_pair(key, val));
	tree->save_base();
}

void forest::Tree::erase(tree_t::key_type key)
{
	tree->erase(key);
	tree->save_base();
}

forest::file_data_t forest::Tree::find(tree_t::key_type key)
{
	auto it = tree->find(key);
	if(it == tree->end()){
		throw DBException(DBException::ERRORS::LEAF_DOES_NOT_EXISTS);
	}
	return it->second;
}


///////////////////////////////////////////////////////////////////////////


void forest::Tree::seed_tree(DBFS::File* f, TREE_TYPES type, int factor)
{
	if(f->fail()){
		delete f;
		throw DBException(DBException::ERRORS::CANNOT_CREATE_FILE);
		return;
	}
	f->write("0 " + std::to_string(DEFAULT_FACTOR) + " " + std::to_string((int)type) + " " + LEAF_NULL + " " + std::to_string((int)NODE_TYPES::LEAF) + "\n");
	f->close();
	delete f;
}


forest::Tree::tree_base_read_t forest::Tree::read_base(string filename)
{
	/*
	if(!DBFS::exists(filename)){
		std::cout << "FAIL WITH NOT EXISTS!!! " << filename << std::endl;
		throw "WTF???";
	}
	*/
	tree_base_read_t ret;
	DBFS::File* f = new DBFS::File(filename);
	/*
	if(f->fail()){
		std::cout << "FAIL WITH OPEN - OTHER!!! " << filename << std::endl;
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
		std::cout << "FAIL WITH OPEN (READ_BASE)!!! " << filename << std::endl;
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

void forest::Tree::materialize_intr(tree_t::node_ptr& node, string path)
{
	tree_t::node_ptr n = get_intr(path);
	cache::intr_cache_r[path].second++;
	node->set_keys(n->get_keys());
	node->set_nodes(n->get_nodes());
}

void forest::Tree::materialize_leaf(tree_t::node_ptr& node, string path)
{
	tree_t::node_ptr n = get_leaf(path), next_leaf, prev_leaf;
	cache::leaf_cache_r[path].second++;
	
	// Lock original node as well
	n->lock();
	
	node->set_childs(n->get_childs());
	
	next_leaf = tree_t::node_ptr(new tree_t::LeafNode());
	prev_leaf = tree_t::node_ptr(new tree_t::LeafNode());
	
	node_data_ptr data = get_node_data(n->data);
	
	next_leaf->data = create_node_data(true, data->next);
	prev_leaf->data = create_node_data(true, data->prev);
	
	if(data->prev != LEAF_NULL){
		node->set_prev_leaf(prev_leaf);
	}
	if(data->next != LEAF_NULL){
		node->set_next_leaf(next_leaf);
	}
}

void forest::Tree::unmaterialize_intr(tree_t::node_ptr& node, string path)
{
	node->set_keys(nullptr);
	node->set_nodes(nullptr);
	cache::intr_cache_r[path].second--;
	cache::check_intr_ref(path);
}

void forest::Tree::unmaterialize_leaf(tree_t::node_ptr& node, string path)
{
	// Unlock original node if it was not already deleted
	if(cache::leaf_cache_r.count(path)){
		tree_t::node_ptr n = get_leaf(path);
		n->unlock();
	}
	
	node->set_childs(nullptr);
	node->set_prev_leaf(nullptr);
	node->set_next_leaf(nullptr);
	cache::leaf_cache_r[path].second--;
	cache::check_leaf_ref(path);
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
		node->data = create_node_data(true, path);
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

forest::Tree::node_data_ptr forest::Tree::get_node_data(void_shared d)
{
	return std::static_pointer_cast<node_data_t>(d);
}

forest::tree_t::node_ptr forest::Tree::get_intr(string path)
{	
	node_ptr intr_data;
	
	// Check cache
	cache::intr_cache_m.lock();
	if(cache::intr_cache.has(path)){
		intr_data = cache::intr_cache.get(path);
		cache::intr_cache_m.unlock();
		return intr_data;
	}
	
	// Check reference
	if(cache::intr_cache_r.count(path)){
		intr_data = cache::intr_cache_r[path].first;
		cache::intr_cache.push(path, intr_data);
		cache::intr_cache_m.unlock();
		return intr_data;
	}
	
	// Check future
	if(cache::intr_cache_q.count(path)){
		std::shared_future<node_ptr> f = cache::intr_cache_q[path];
		cache::intr_cache_m.unlock();
		intr_data = f.get();
		return intr_data;
	}
	
	// Read intr node data
	std::promise<node_ptr> p;
	cache::intr_cache_q[path] = p.get_future();
	cache::intr_cache_m.unlock();
	
	// Fill node
	intr_data = node_ptr(new tree_t::InternalNode());
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
		n->data = create_node_data(true, child_path);
		intr_data->add_nodes(i,n);
	}
	intr_data->data = create_node_data(false, path);
	
	// Set cache and future
	cache::intr_cache_m.lock();
	cache::intr_cache.push(path, intr_data);
	cache::intr_cache_r[path] = std::make_pair(intr_data,0);
	p.set_value(intr_data);
	cache::intr_cache_q.erase(path);
	cache::intr_cache_m.unlock();
	
	// Return
	return intr_data;
}

forest::tree_t::node_ptr forest::Tree::get_leaf(string path)
{
	using entry_pair = std::pair<const tree_t::key_type, tree_t::val_type>;
	using entry_ptr = std::shared_ptr<entry_pair>;
	
	node_ptr leaf_data;
	
	cache::leaf_cache_m.lock();
	
	// Check cache
	if(cache::leaf_cache.has(path)){
		leaf_data = cache::leaf_cache.get(path);
		cache::leaf_cache_m.unlock();
		return leaf_data;
	}
	
	// Check reference
	if(cache::leaf_cache_r.count(path)){
		leaf_data = cache::leaf_cache_r[path].first;
		cache::leaf_cache.push(path, leaf_data);
		cache::leaf_cache_m.unlock();
		return leaf_data;
	}
	
	// Check future
	if(cache::leaf_cache_q.count(path)){
		std::shared_future<node_ptr> f = cache::leaf_cache_q[path];
		cache::leaf_cache_m.unlock();
		leaf_data = f.get();
		return leaf_data;
	}
	
	// Read leaf node data
	std::promise<node_ptr> p;
	cache::leaf_cache_q[path] = p.get_future();
	cache::leaf_cache_m.unlock();
	
	// Fill node
	leaf_data = node_ptr(new typename tree_t::LeafNode());
	tree_leaf_read_t leaf_d = read_leaf(path);
	std::vector<tree_t::key_type>* keys_ptr = leaf_d.child_keys;
	std::vector<uint_t>* vals_length = leaf_d.child_lengths;
	uint_t start_data = leaf_d.start_data;
	std::shared_ptr<DBFS::File> f(leaf_d.file);
	int c = keys_ptr->size();
	uint_t last_len = 0;
	for(int i=0;i<c;i++){
		leaf_data->insert(entry_ptr(new entry_pair( (*keys_ptr)[i], file_data_t(start_data+last_len, (*vals_length)[i], f, [](file_data_t* self, char* buf, int sz){ 
			try{
				self->file->seek(self->curr);
				self->file->read(buf, sz);
			} catch(...) {
				//std::cout << "ERROR WHEN SOMETHING ELSE" << std::endl;
				//throw "error";
			}
		}) )));
		last_len += (*vals_length)[i];
	}
	//leaf_data->update_positions(leaf_data);
	
	leaf_data->data = create_node_data(false, path, leaf_d.left_leaf, leaf_d.right_leaf);
	
	// Set cache and future
	cache::leaf_cache_m.lock();
	cache::leaf_cache.push(path, leaf_data);
	cache::leaf_cache_r[path] = std::make_pair(leaf_data,0);
	p.set_value(leaf_data);
	cache::leaf_cache_q.erase(path);
	cache::leaf_cache_m.unlock();
	
	// Return
	return leaf_data;
}

void forest::Tree::write_intr(DBFS::File* file, tree_intr_read_t data)
{
	auto* keys = data.child_keys;
	auto* paths = data.child_values;
	file->write( std::to_string((int)data.childs_type) + " " + std::to_string(paths->size()) + "\n" );
	
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
	file->write( std::to_string(data.count) + " " + std::to_string(data.factor) + " " + std::to_string((int)data.type) + " " + data.branch + " " + std::to_string((int)data.branch_type) + "\n" );
}

void forest::Tree::write_leaf(std::shared_ptr<DBFS::File> file, tree_leaf_read_t data)
{
	auto* keys = data.child_keys;
	auto* lengths = data.child_lengths;
	int c = keys->size();
	file->write(std::to_string(c) + " " + data.left_leaf + " " + data.right_leaf+ "\n");
	string lenStr = "";
	std::stringstream ss;
	for(auto& key : (*keys)){
		ss << key << " ";
	}
	for(auto& len : (*lengths)){
		lenStr.append(std::to_string(len) + " ");
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
	data.reset();
	try{
		while( (rsz = data.read(buf, read_size)) ){
			file->write(buf, rsz);
		}
	} catch(...){
		//std::cout << "ERROR WHEN WRITE LEAF ITEM" << std::endl;
		//throw "error";
	}
	delete[] buf;
	data.start = start_data;
	data.reset();
	data.file = file;
	data._read = [](file_data_t* self, char* buf, int sz){ 
		self->file->seek(self->curr);
		self->file->read(buf, sz);
	};
}

void forest::Tree::clear_node_cache(tree_t::node_ptr node)
{
	node_data_ptr data = get_node_data(node->data);
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
		[this](tree_t::node_ptr& node, tree_t* tree){ this->d_enter(node, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_leave(node, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_insert(node, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_remove(node, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_reserve(node, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_release(node, tree); }
		,[this](tree_t::child_item_type_ptr item, int_t step, tree_t* tree){ this->d_before_move(item, step, tree); }
		,[this](tree_t::child_item_type_ptr item, int_t step, tree_t* tree){ this->d_after_move(item, step, tree); }
		,[this](tree_t::node_ptr& node, tree_t* tree){ this->d_save_base(node, tree); }
	);
}



// Proceed
void forest::Tree::d_enter(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "ENTER_START" << std::endl;
		
	// Lock node mutex
	node->lock();
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		//std::cout << "ENTER_END - STEM" << std::endl;
		return;
	}
	
	// Check for newly created node
	if(!node->data){
		//std::cout << "ENTER_END - NODATA" << std::endl;
		return;
	}
		
	// Check for node already materialized
	node_data_ptr data = get_node_data(node->data);
	if(!data->ghost)
		return;
		
	if(!node->is_leaf()){
		materialize_intr(node, data->path);
	}
	else{
		materialize_leaf(node, data->path);
	}
	data->ghost = false;
	
	//std::cout << "ENTER_END" << std::endl;
}

void forest::Tree::d_leave(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "LEAVE_START" << std::endl;
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		node->unlock();
		//std::cout << "LEAVE_END - STEM" << std::endl;
		return;
	}
	
	if(!node->data){
		node->unlock();
		//std::cout << "LEAVE_END - NODATA" << std::endl;
		return;
	}
	
	node_data_ptr data = get_node_data(node->data);
	if(!node->is_leaf()){
		unmaterialize_intr(node, data->path);
	}
	else{
		unmaterialize_leaf(node, data->path);
	}
	data->ghost = true;
	node->unlock();
	
	//std::cout << "LEAVE_END" << std::endl;
}

void forest::Tree::d_insert(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "INSERT_START" << std::endl;
	
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
			node_data_ptr d = get_node_data( (*(node->get_nodes()))[i]->data );
			(*nodes)[i] = d->path;
		}
		intr_d.child_keys = keys;
		intr_d.child_values = nodes;
		write_intr(f, intr_d);
		
		f->close();
		
		if(!node->data){
			node->data = create_node_data(false, new_name);
			// Cache new intr node
			tree_t::node_ptr n = tree_t::node_ptr(new tree_t::InternalNode());
			n->set_keys(node->get_keys());
			n->set_nodes(node->get_nodes());
			cache::intr_cache_m.lock();
			cache::intr_cache.push(new_name, n);
			cache::intr_cache_r[new_name] = make_pair(n, 1);
			cache::intr_cache_m.unlock();
		} else {
			node_data_ptr data = get_node_data(node->data);
			string cur_name = data->path;
			DBFS::remove(cur_name);
			DBFS::move(new_name, cur_name);
		}
	} else {
		tree_leaf_read_t leaf_d;
		auto* keys = new std::vector<tree_t::key_type>();
		auto* lengths = new std::vector<uint_t>();
		
		node_ptr p_leaf = node->prev_leaf();
		node_ptr n_leaf = node->next_leaf();
		
		//!p_leaf = nullptr;
		//!n_leaf = nullptr;
		
		if(p_leaf){
			assert(p_leaf->data);
			node_data_ptr p_leaf_data = get_node_data(p_leaf->data);
			leaf_d.left_leaf = p_leaf_data->path;
		} else {
			leaf_d.left_leaf = LEAF_NULL;
		}
		if(n_leaf){
			assert(n_leaf->data);
			node_data_ptr n_leaf_data = get_node_data(n_leaf->data);
			leaf_d.right_leaf = n_leaf_data->path;
		} else {
			leaf_d.right_leaf = LEAF_NULL;
		}
		
		for(auto& it : *(node->get_childs()) ){
			keys->push_back(it->item->first);
			lengths->push_back(it->item->second.size());
		}
		
		leaf_d.child_keys = keys;
		leaf_d.child_lengths = lengths;
		std::shared_ptr<DBFS::File> fp(f);
		write_leaf(fp, leaf_d);
		for(auto& it : *(node->get_childs()) ){
			write_leaf_item(fp, it->item->second);
		}
		
		fp->stream().flush();
		
		if(!node->data){
			node->data = create_node_data(false, new_name);
			// Cache new leaf node
			node_ptr n = tree_t::node_ptr(new tree_t::LeafNode());
			n->set_childs(node->get_childs());
			
			n->data = create_node_data(false, new_name, leaf_d.left_leaf, leaf_d.right_leaf);
			
			//n->update_positions(n);
			cache::leaf_cache_m.lock();
			cache::leaf_cache.push(new_name, n);
			cache::leaf_cache_r[new_name] = make_pair(n, 1);
			cache::leaf_cache_m.unlock();
		} else {
			node_data_ptr data = get_node_data(node->data);
			string cur_name = data->path;
			
			cache::leaf_cache_m.lock();
			if(cache::leaf_cache_r.count(cur_name)){
				node_ptr n = cache::leaf_cache_r[cur_name].first;
				
				node_data_ptr c_data = get_node_data(n->data);
				c_data->prev = leaf_d.left_leaf;
				c_data->next = leaf_d.right_leaf;
				
				//n->update_positions(n);
			}
			cache::leaf_cache_m.unlock();
			
			DBFS::remove(cur_name);
			fp->move(cur_name);
		}
	}
	
	//std::cout << "INSERT_END" << std::endl;
}

void forest::Tree::d_remove(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "REMOVE_START" << std::endl;
	
	if(!node->data){
		//std::cout << "REMOVE_END - NODATA" << std::endl;
		return;
	}
	
	node_data_ptr data = get_node_data(node->data);
	
	if(node->is_leaf() && node->size()){
		node->first_child()->item->second.file->close();
		node->get_childs()->resize(0);
	}
	
	DBFS::remove(data->path);
	
	clear_node_cache(node);
	
	//std::cout << "REMOVE_END" << std::endl;
}

void forest::Tree::d_reserve(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "RESERVE" << std::endl;
}

void forest::Tree::d_release(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "RELEASE" << std::endl;
}

void forest::Tree::d_before_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{
	//std::cout << "BEFORE_MOVE" << std::endl;
}

void forest::Tree::d_after_move(tree_t::child_item_type_ptr item, int_t step, tree_t* tree)
{
	//std::cout << "AFTER_MOVE" << std::endl;
}

void forest::Tree::d_save_base(tree_t::node_ptr& node, tree_t* tree)
{
	//std::cout << "SAVE_BASE_START" << std::endl;
	
	// Save Base File
	string base_file_name = this->get_name();
	DBFS::File* base_f = DBFS::create();
	tree_base_read_t base_d;
	base_d.type = this->get_type();
	base_d.count = tree->size();
	base_d.factor = tree->get_factor();
	
	tree_t::node_ptr root_node = tree->get_root_pub();
	base_d.branch_type = (root_node->is_leaf() ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	node_data_ptr base_data = get_node_data(root_node->data);
	base_d.branch = base_data->path;
	write_base(base_f, base_d);
	string new_base_file_name = base_f->name();
	
	base_f->close();
	DBFS::remove(base_file_name);
	DBFS::move(new_base_file_name, base_file_name);
	
	//std::cout << "SAVE_BASE_END" << std::endl;
}

