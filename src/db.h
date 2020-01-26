#ifndef DB_H
#define DB_H

#include <iostream>

#include <string>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <memory>
#include <mutex>
#include <future>
#include <vector>
#include <type_traits>
#include <atomic>
#include <sstream>
#include <cassert>
#include "bplustree.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"
#include "dbdriver.hpp"

class DB{
	
	enum class KEY_TYPES { INT, STRING };
	enum class NODE_TYPES { INTR, LEAF };
	
	using string = std::string;
	using int_t = long long int;
	using file_pos_t = long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	
	using driver_root_t = DBDriver<string, file_data_t>;
	using driver_int_t = DBDriver<int_t, file_data_t>;
	using driver_string_t = DBDriver<string, file_data_t>;
	
	using root_tree_t = BPlusTree<string, file_data_t, driver_root_t>;
	using int_tree_t = BPlusTree<int_t, file_data_t, driver_int_t>;
	using string_tree_t = BPlusTree<string, file_data_t, driver_string_t>;
	
	using int_a = std::atomic<int>;
	
	struct tree_t {
		void_shared tree;
		TREE_TYPES type;
	};
	struct leaf_t {
		TREE_TYPES type;
		void* child_keys;
		void* child_lengths;
		int_t start_data;
		DBFS::File* file;
		string left_leaf, right_leaf;
	};
	struct intr_t {
		TREE_TYPES type;
		NODE_TYPES childs_type;
		void* child_keys;
		void* child_values;
	};
	struct tree_base_read_t {
		int_t count;
		int factor;
		TREE_TYPES type;
		string branch;
		NODE_TYPES branch_type;
	};
	struct node_data_t {
		bool ghost = true;
		string path = "";
		string prev = "-";
		string next = "-";
		node_data_t(bool ghost, string path) : ghost(ghost), path(path) {};
	};
	
	using node_data_ptr = std::shared_ptr<node_data_t>;

	public:
		DB();
		~DB();
		DB(string path);
		
		void create_tree(TREE_TYPES type, string name);
		void delete_tree(string name);
		tree_t find_tree(string name);
		tree_t open_tree(string path);
		void close_tree(string path);
		
		//void insert_leaf(string name);
		//void insert_leaf(tree_t tree);
		//void erase_leaf(string name);
		//void erase_leaf(tree_t tree);
		//find_leaf();
				
		void bloom(string path);
		void fold();
		
	private:
		root_tree_t* FOREST;
		bool blossomed = false;
		
		// Root methods
		void open_root();
		void close_root();
		
		// Intr methods
		template<typename T>
		intr_t read_intr(string filename);
		template<typename T>
		void materialize_intr(typename T::node_ptr& node, string path);
		template<typename T>
		void unmaterialize_intr(typename T::node_ptr& node, string path);
		void check_intr_ref(string key);
		
		// Leaf methods
		template<typename T>
		leaf_t read_leaf(string filename);
		template<typename T>
		void materialize_leaf(typename T::node_ptr& node, string path);
		template<typename T>
		void unmaterialize_leaf(typename T::node_ptr& node, string path);
		void check_leaf_ref(string key);
		
		// Tree base methods
		tree_base_read_t read_base(string filename);
		void create_root_file();
		string create_tree_base(TREE_TYPES type);
		void insert_tree(string name, string file_name, TREE_TYPES type);
		void erase_tree(string path);
		void check_tree_ref(string key);
		
		// Create Tree Node smart ptr
		template<typename T>
		typename T::node_ptr create_node(string path, NODE_TYPES node_type);
		
		// Node data
		node_data_ptr create_node_data(bool ghost, string path);
		node_data_ptr create_node_data(bool ghost, string path, string prev, string next);
		node_data_ptr get_node_data(void_shared d);
		
		// Cache
		void init_cache();
		void release_cache();
		template<typename T>
		void clear_node_cache(typename T::node_ptr node);
		ListCache<string, tree_t> tree_cache;
		ListCache<string, void_shared> leaf_cache;
		ListCache<string, void_shared> intr_cache;
		mutex tree_cache_m, leaf_cache_m, intr_cache_m;
		std::unordered_map<string, std::shared_future<tree_t> > tree_cache_q;
		std::unordered_map<string, std::shared_future<void_shared> > intr_cache_q, leaf_cache_q;
		std::unordered_map<string, std::pair<tree_t, int_a> > tree_cache_r;
		std::unordered_map<string, std::pair<void_shared, int_a> > intr_cache_r, leaf_cache_r;
		std::unordered_map<int_t, string> tree_cache_f;
		// std::unordered_set<string> tree_cache_t;
		
		// Drivers
		void init_drivers();
		void release_drivers();
		driver_root_t* driver_root;
		driver_int_t* driver_int;
		driver_string_t* driver_string;
		
		// Proceed
		template<typename T>
		void d_enter(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_leave(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_insert(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_remove(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_reserve(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_release(typename T::node_ptr& node, T* tree);
		template<typename T>
		void d_before_move(typename T::child_item_type_ptr item, int_t step, T* tree);
		template<typename T>
		void d_after_move(typename T::child_item_type_ptr item, int_t step, T* tree);
		
		// Getters
		template<typename T>
		typename T::node_ptr get_intr(string path);
		template<typename T>
		typename T::node_ptr get_leaf(string path);
		tree_t get_tree(string path);
		
		// Writers
		template<typename T>
		void write_intr(DBFS::File* file, intr_t data);
		template<typename T>
		void write_base(DBFS::File* file, tree_base_read_t data);
		template<typename T>
		void write_leaf(std::shared_ptr<DBFS::File> file, leaf_t data);
		template<typename T>
		void write_leaf_item(std::shared_ptr<DBFS::File> file, typename T::val_type& data);
		
		// Other
		template<typename T>
		TREE_TYPES get_tree_type();
		template<typename T>
		typename T::node_ptr unvoid_node(void_shared node);
		string read_leaf_item(file_data_t item);
	
		// Properties
		const int DEFAULT_FACTOR = 3;
		const string ROOT_TREE = "_root";
		const string LEAF_NULL = "-";
		
		std::mutex tempmtx;
};

template<typename T>
typename T::node_ptr DB::create_node(string path, NODE_TYPES node_type)
{
	typename T::Node* node;
	if(node_type == NODE_TYPES::INTR){
		node = new typename T::InternalNode();
	} else {
		node = new typename T::LeafNode();
	}
	node->data = create_node_data(true, path);
	return typename T::node_ptr(node);
}

template<typename T>
void DB::d_enter(typename T::node_ptr& node, T* tree)
{	
	//std::cout << "ENTER_START" << std::endl;
		
	// Lock node mutex
	node->lock();
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node))
		return;
	
	// Check for newly created node
	if(!node->data)
		return;
		
	// Check for node already materialized
	node_data_ptr data = get_node_data(node->data);
	if(!data->ghost)
		return;
		
	if(!node->is_leaf()){
		materialize_intr<T>(node, data->path);
		//TODO: Lock shared node
	}
	else{
		materialize_leaf<T>(node, data->path);
		//TODO: Lock shared node
	}
	data->ghost = false;
	
	//std::cout << "ENTER_END" << std::endl;
}

template<typename T>
void DB::d_leave(typename T::node_ptr& node, T* tree)
{
	//std::cout << "LEAVE_START" << std::endl;
	
	// Nothing to do with stem
	if(tree->is_stem_pub(node)){
		node->unlock();
		return;
	}
	
	node_data_ptr data = get_node_data(node->data);
	if(!node->is_leaf()){
		unmaterialize_intr<T>(node, data->path);
		//TODO: Unlock shared node
	}
	else{
		unmaterialize_leaf<T>(node, data->path);
		//TODO: Unlock shared node
	}
	data->ghost = true;
	node->unlock();
	
	//std::cout << "LEAVE_END" << std::endl;
}

template<typename T>
void DB::d_insert(typename T::node_ptr& node, T* tree)
{	
	//std::cout << "INSERT_START" << std::endl;
	
	if(tree->is_stem_pub(node)){
		assert(false);
	}
	
	DBFS::File* f = DBFS::create();
	string new_name = f->name();
	
	if(!node->is_leaf()){
		intr_t intr_d;
		intr_d.type = get_tree_type<T>();
		intr_d.childs_type = ((node->first_child_node()->is_leaf()) ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
		int c = node->get_nodes()->size();
		auto* keys = new std::vector<typename T::key_type>(node->keys_iterator(), node->keys_iterator_end());
		auto* nodes = new std::vector<string>(c);
		for(int i=0;i<c;i++){
			node_data_ptr d = get_node_data( (*(node->get_nodes()))[i]->data );
			(*nodes)[i] = d->path;
		}
		intr_d.child_keys = keys;
		intr_d.child_values = nodes;
		write_intr<T>(f, intr_d);
		
		f->close();
		
		if(!node->data){
			node->data = create_node_data(false, new_name);
			// Cache new intr node
			typename T::node_ptr n = typename T::node_ptr(new typename T::InternalNode());
			n->set_keys(node->get_keys());
			n->set_nodes(node->get_nodes());
			intr_cache_m.lock();
			intr_cache.push(new_name, n);
			intr_cache_r[new_name] = make_pair(n, 1);
			intr_cache_m.unlock();
		} else {
			node_data_ptr data = get_node_data(node->data);
			string cur_name = data->path;
			DBFS::remove(cur_name);
			DBFS::move(new_name, cur_name);
		}
	} else {
		leaf_t leaf_d;
		leaf_d.type = get_tree_type<T>();
		auto* keys = new std::vector<typename T::key_type>();
		auto* lengths = new std::vector<int_t>();
		
		typename T::node_ptr p_leaf = node->prev_leaf();
		typename T::node_ptr n_leaf = node->next_leaf();
		
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
		write_leaf<T>(fp, leaf_d);
		for(auto& it : *(node->get_childs()) ){
			write_leaf_item<T>(fp, it->item->second);
		}
		
		fp->stream().flush();
		
		if(!node->data){
			node->data = create_node_data(false, new_name);
			// Cache new leaf node
			typename T::node_ptr n = typename T::node_ptr(new typename T::LeafNode());
			n->set_childs(node->get_childs());
			
			n->data = create_node_data(false, new_name, leaf_d.left_leaf, leaf_d.right_leaf);
			
			//n->update_positions(n);
			leaf_cache_m.lock();
			leaf_cache.push(new_name, n);
			leaf_cache_r[new_name] = make_pair(n, 1);
			leaf_cache_m.unlock();
		} else {
			node_data_ptr data = get_node_data(node->data);
			string cur_name = data->path;
			
			leaf_cache_m.lock();
			if(leaf_cache_r.count(cur_name)){
				typename T::node_ptr n = unvoid_node<T>(leaf_cache_r[cur_name].first);
				
				node_data_ptr c_data = get_node_data(n->data);
				c_data->prev = leaf_d.left_leaf;
				c_data->next = leaf_d.right_leaf;
				
				//n->update_positions(n);
			}
			leaf_cache_m.unlock();
			
			DBFS::remove(cur_name);
			fp->move(cur_name);
		}
	}
	
	/*
	// Save Base File
	tempmtx.lock();
	string base_file_name = tree_cache_f[(int_t)tree];
	DBFS::File* base_f = DBFS::create();
	tree_base_read_t base_d;
	base_d.type = get_tree_type<T>();
	base_d.count = tree->size();
	base_d.factor = tree->get_factor();
	
	typename T::node_ptr root_node = tree->get_root_pub();
	base_d.branch_type = (root_node->is_leaf() ? NODE_TYPES::LEAF : NODE_TYPES::INTR);
	node_data_ptr base_data = get_node_data(root_node->data);
	base_d.branch = base_data->path;
	write_base<T>(base_f, base_d);
	string new_base_file_name = base_f->name();
	
	base_f->close();
	DBFS::remove(base_file_name);
	DBFS::move(new_base_file_name, base_file_name);
	
	tempmtx.unlock();*/
	//std::cout << "INSERT_END" << std::endl;
}

template<typename T>
void DB::d_remove(typename T::node_ptr& node, T* tree)
{
	//std::cout << "REMOVE_START" << std::endl;
	
	node_data_ptr data = get_node_data(node->data);
	
	if(node->is_leaf() && node->size()){
		node->first_child()->item->second.file->close();
		node->get_childs()->resize(0);
	}
	
	DBFS::remove(data->path);
	
	clear_node_cache<T>(node);
	
	//std::cout << "REMOVE_END" << std::endl;
}

template<typename T>
void DB::d_reserve(typename T::node_ptr& node, T* tree)
{
	//std::cout << "RESERVE" << std::endl;
}

template<typename T>
void DB::d_release(typename T::node_ptr& node, T* tree)
{
	//std::cout << "RELEASE" << std::endl;
}

template<typename T>
void DB::d_before_move(typename T::child_item_type_ptr item, int_t step, T* tree)
{
	//std::cout << "BEFORE_MOVE" << std::endl;
}

template<typename T>
void DB::d_after_move(typename T::child_item_type_ptr item, int_t step, T* tree)
{
	//std::cout << "AFTER_MOVE" << std::endl;
}

template<typename T>
void DB::materialize_intr(typename T::node_ptr& node, string path)
{
	typename T::node_ptr n = get_intr<T>(path);
	node->set_keys(n->get_keys());
	node->set_nodes(n->get_nodes());
	intr_cache_r[path].second++;
}

template<typename T>
void DB::materialize_leaf(typename T::node_ptr& node, string path)
{
	typename T::node_ptr n = get_leaf<T>(path), next_leaf, prev_leaf;
	node->set_childs(n->get_childs());
	
	next_leaf = typename T::node_ptr(new typename T::LeafNode());
	prev_leaf = typename T::node_ptr(new typename T::LeafNode());
	
	node_data_ptr data = get_node_data(n->data);
	
	next_leaf->data = create_node_data(true, data->next);
	prev_leaf->data = create_node_data(true, data->prev);
	
	if(data->prev != LEAF_NULL){
		node->set_prev_leaf(prev_leaf);
	}
	if(data->next != LEAF_NULL){
		node->set_next_leaf(next_leaf);
	}
	
	leaf_cache_r[path].second++;
}

template<typename T>
void DB::unmaterialize_intr(typename T::node_ptr& node, string path)
{
	intr_cache_r[path].second--;
	node->set_keys(nullptr);
	node->set_nodes(nullptr);
	check_intr_ref(path);
}

template<typename T>
void DB::unmaterialize_leaf(typename T::node_ptr& node, string path)
{
	leaf_cache_r[path].second--;
	node->set_childs(nullptr);
	node->set_prev_leaf(nullptr);
	node->set_next_leaf(nullptr);
	check_leaf_ref(path);
}

template<typename T>
typename T::node_ptr DB::get_intr(string path)
{
	using node_ptr = typename T::node_ptr;
	
	node_ptr intr_data;
	
	// Check cache
	intr_cache_m.lock();
	if(intr_cache.has(path)){
		intr_data = unvoid_node<T>(intr_cache.get(path));
		intr_cache_m.unlock();
		return intr_data;
	}
	
	// Check reference
	if(intr_cache_r.count(path)){
		intr_data = unvoid_node<T>(intr_cache_r[path].first);
		intr_cache.push(path, intr_data);
		intr_cache_m.unlock();
		return intr_data;
	}
	
	// Check future
	if(intr_cache_q.count(path)){
		std::shared_future<void_shared> f = intr_cache_q[path];
		intr_cache_m.unlock();
		intr_data = unvoid_node<T>(f.get());
		return intr_data;
	}
	
	// Read intr node data
	std::promise<void_shared> p;
	intr_cache_q[path] = p.get_future();
	intr_cache_m.unlock();
	
	// Fill node
	intr_data = node_ptr(new typename T::InternalNode());
	intr_t intr_d = read_intr<T>(path);
	std::vector<typename T::key_type>* keys_ptr = (std::vector<typename T::key_type>*)intr_d.child_keys;
	std::vector<string>* vals_ptr = (std::vector<string>*)intr_d.child_values;
	intr_data->add_keys(0, keys_ptr->begin(), keys_ptr->end());
	int c = vals_ptr->size();
	for(int i=0;i<c;i++){
		typename T::node_ptr n;
		string& child_path = (*vals_ptr)[i];
		if(intr_d.childs_type == NODE_TYPES::INTR){
			n = typename T::node_ptr(new typename T::InternalNode());
		} else {
			n = typename T::node_ptr(new typename T::LeafNode());
		}
		n->data = create_node_data(true, child_path);
		intr_data->add_nodes(i,n);
	}
	intr_data->data = create_node_data(false, path);
	
	// Set cache and future
	intr_cache_m.lock();
	intr_cache.push(path, intr_data);
	intr_cache_r[path] = std::make_pair(intr_data,0);
	p.set_value(intr_data);
	intr_cache_q.erase(path);
	intr_cache_m.unlock();
	
	// Return
	return intr_data;
}

template<typename T>
typename T::node_ptr DB::get_leaf(string path)
{
	using node_ptr = typename T::node_ptr;
	using entry_pair = std::pair<const typename T::key_type, typename T::val_type>;
	using entry_ptr = std::shared_ptr<entry_pair>;
	
	node_ptr leaf_data;
	
	leaf_cache_m.lock();
	
	// Check cache
	if(leaf_cache.has(path)){
		leaf_data = unvoid_node<T>(leaf_cache.get(path));
		leaf_cache_m.unlock();
		return leaf_data;
	}
	
	// Check reference
	if(leaf_cache_r.count(path)){
		leaf_data = unvoid_node<T>(leaf_cache_r[path].first);
		leaf_cache.push(path, leaf_data);
		leaf_cache_m.unlock();
		return leaf_data;
	}
	
	// Check future
	if(leaf_cache_q.count(path)){
		std::shared_future<void_shared> f = leaf_cache_q[path];
		leaf_cache_m.unlock();
		leaf_data = unvoid_node<T>(f.get());
		return leaf_data;
	}
	
	// Read leaf node data
	std::promise<void_shared> p;
	leaf_cache_q[path] = p.get_future();
	leaf_cache_m.unlock();
	
	// Fill node
	leaf_data = node_ptr(new typename T::LeafNode());
	leaf_t leaf_d = read_leaf<T>(path);
	std::vector<typename T::key_type>* keys_ptr = (std::vector<typename T::key_type>*)leaf_d.child_keys;
	std::vector<int_t>* vals_length = (std::vector<int_t>*)leaf_d.child_lengths;
	int_t start_data = leaf_d.start_data;
	std::shared_ptr<DBFS::File> f(leaf_d.file);
	int c = keys_ptr->size();
	int_t last_len = 0;
	for(int i=0;i<c;i++){
		leaf_data->insert(entry_ptr(new entry_pair( (*keys_ptr)[i], file_data_t(start_data+last_len, (*vals_length)[i], f, [](file_data_t* self, char* buf, int sz){ 
			try{
				self->file->seek(self->curr);
				self->file->read(buf, sz);
			} catch(...) {
				std::cout << "ERROR WHEN SOMETHING ELSE" << std::endl;
				throw "error";
			}
		}) )));
		last_len += (*vals_length)[i];
	}
	//leaf_data->update_positions(leaf_data);
	
	leaf_data->data = create_node_data(false, path, leaf_d.left_leaf, leaf_d.right_leaf);
	
	// Set cache and future
	leaf_cache_m.lock();
	leaf_cache.push(path, leaf_data);
	leaf_cache_r[path] = std::make_pair(leaf_data,0);
	p.set_value(leaf_data);
	leaf_cache_q.erase(path);
	leaf_cache_m.unlock();
	
	// Return
	return leaf_data;
}

template<typename T>
DB::intr_t DB::read_intr(string filename)
{
	TREE_TYPES type = get_tree_type<T>();
	using key_type = typename T::key_type;
	
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
	
	return {type, (NODE_TYPES)t, keys, vals};
}

template<typename T>
DB::leaf_t DB::read_leaf(string filename)
{	
	DBFS::File* f = new DBFS::File(filename);
	
	int c;
	string left_leaf, right_leaf;
	int_t start_data;
	
	f->read(c);
	f->read(left_leaf);
	f->read(right_leaf);
	
	auto* keys = new std::vector<typename T::key_type>(c);
	auto* vals_lengths = new std::vector<int_t>(c);
	for(int i=0;i<c;i++){
		f->read((*keys)[i]);
	}
	for(int i=0;i<c;i++){
		f->read((*vals_lengths)[i]);
	}
	start_data = f->tell()+1;
	leaf_t t;
	t.type = get_tree_type<T>();
	t.child_keys = keys;
	t.child_lengths = vals_lengths;
	t.left_leaf = left_leaf;
	t.right_leaf = right_leaf;
	t.start_data = start_data;
	t.file = f;
	return t;
}

template<typename T>
void DB::write_base(DBFS::File* file, tree_base_read_t data)
{
	file->write( std::to_string(data.count) + " " + std::to_string(data.factor) + " " + std::to_string((int)data.type) + " " + data.branch + " " + std::to_string((int)data.branch_type) + "\n" );
}

template<typename T>
void DB::write_intr(DBFS::File* file, intr_t data)
{
	auto* keys = (std::vector<typename T::key_type>*)data.child_keys;
	auto* paths = (std::vector<string>*)data.child_values;
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

template<typename T>
void DB::write_leaf(std::shared_ptr<DBFS::File> file, leaf_t data)
{
	auto* keys = (std::vector<typename T::key_type>*)data.child_keys;
	auto* lengths = (std::vector<int_t>*)data.child_lengths;
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

template<typename T>
void DB::write_leaf_item(std::shared_ptr<DBFS::File> file, typename T::val_type& data)
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
		std::cout << "ERROR WHEN WRITE LEAF ITEM" << std::endl;
		throw "error";
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

template<typename T>
void DB::clear_node_cache(typename T::node_ptr node)
{
	node_data_ptr data = get_node_data(node->data);
	string path = data->path;
	if(node->is_leaf()){
		leaf_cache_m.lock();
		if(leaf_cache_r.count(path)){
			leaf_cache_r.erase(path);
		}
		if(leaf_cache.has(path)){
			leaf_cache.remove(path);
		}
		leaf_cache_m.unlock();
	}
	else{
		intr_cache_m.lock();
		if(intr_cache_r.count(path)){
			intr_cache_r.erase(path);
		}
		if(intr_cache.has(path)){
			intr_cache.remove(path);
		}
		intr_cache_m.unlock();
	}
}

template<typename T>
TREE_TYPES DB::get_tree_type()
{
	if(std::is_same<T,int_tree_t>::value)
		return TREE_TYPES::KEY_INT;
	if(std::is_same<T,string_tree_t>::value)
		return TREE_TYPES::KEY_STRING;
	return TREE_TYPES::ROOT;
}

template<typename T>
typename T::node_ptr DB::unvoid_node(void_shared node)
{
	return std::static_pointer_cast<typename T::Node>(node);
}

#endif // DB_H
