#include "forest.hpp"

namespace forest{
namespace details{

	Savior* savior;
	bool folding = false;

	tree_ptr FOREST;
	bool blossomed = false;

} // details
} // forest


void forest::bloom(details::string path)
{
	L_PUB("[forest::bloom]-start");

	if(blooms()){
		return;
	}

	details::cache::init_cache();

	DBFS::set_root(path);
	if(!DBFS::exists(details::ROOT_TREE)){
		details::create_root_file();
	} 

	details::init_savior();
	details::open_root();

	details::blossomed = true;

	L_PUB("[forest::bloom]-end");
}

void forest::fold()
{
	L_PUB("[forest::fold]-start");

	if(!blooms()){
		return;
	}

	details::folding = true;
	details::blossomed = false;

	details::cache::release_cache();
	details::release_savior();
	details::close_root();

	L_PUB("[forest::fold]-end");
}

bool forest::blooms()
{
	return details::blossomed;
}

int forest::get_save_queue_size()
{
	return details::savior->save_queue_size();
}

int forest::get_opened_files_count()
{
	return details::opened_files_count.load();
}

void forest::plant_tree(TREE_TYPES type, details::string name, int factor, details::string annotation)
{
	L_PUB("[forest::plant_tree]-" + name);

	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	if(!factor){
		factor = details::DEFAULT_FACTOR;
	}

	if(details::FOREST->get_tree()->find(name) != details::FOREST->get_tree()->end()){
		throw TreeException(TreeException::ERRORS::TREE_ALREADY_EXISTS);
	}

	details::string file_name = DBFS::random_filename();

	details::tree_ptr tree = details::tree_ptr(new details::Tree(file_name, type, factor, annotation));

	details::insert_tree(name, file_name, tree);
}

void forest::cut_tree(details::string name)
{
	L_PUB("[forest::cut_tree]-" + name);

	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::string path;
	// Not exist
	{
		auto it = details::FOREST->get_tree()->find(name);
		if(it == details::FOREST->get_tree()->end()){
			return;
		}

		// Get tree path
		path = details::read_leaf_item(it->second);
	}
	
	// Remove tree from forest
	details::FOREST->erase(name);

	// Erase tree
	details::erase_tree(path);
}

forest::Tree forest::find_tree(details::string name)
{
	L_PUB("[forest::find_tree]-" + name);

	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	// Error if not exists
	auto it = details::FOREST->get_tree()->find(name);
	if(it == details::FOREST->get_tree()->end()){
		throw TreeException(TreeException::ERRORS::TREE_DOES_NOT_EXISTS);
	}

	// Get tree path
	details::string path = details::read_leaf_item(it->second);
	return details::tree_owner_ptr(new details::tree_owner(details::reach_tree(path)));
}

void forest::insert_leaf(details::string tree_name, details::tree_t::key_type key, details::detached_leaf_ptr val)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::insert_leaf]-" + t->get_name() + "_" + key);

	t->insert(key, details::extract_leaf_val(val));
}

void forest::insert_leaf(Tree tree, details::tree_t::key_type key, details::detached_leaf_ptr val)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::insert_leaf]-" + t->get_name() + "_" + key);

	t->insert(key, details::extract_leaf_val(val));
}

void forest::update_leaf(details::string tree_name, details::tree_t::key_type key, details::detached_leaf_ptr val)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::update_leaf]-" + t->get_name() + "_" + key);

	t->insert(key, details::extract_leaf_val(val), true);
}

void forest::update_leaf(Tree tree, details::tree_t::key_type key, details::detached_leaf_ptr val)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::update_leaf]-" + t->get_name() + "_" + key);

	details::extract_native_tree(tree)->insert(key, details::extract_leaf_val(val), true);
}

void forest::remove_leaf(details::string tree_name, details::tree_t::key_type key)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::remove_leaf]-" + t->get_name() + "_" + key);

	t->erase(key);
}

void forest::remove_leaf(Tree tree, details::tree_t::key_type key)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr t = details::extract_native_tree(tree);

	L_PUB("[forest::remove_leaf]-" + t->get_name() + "_" + key);

	details::extract_native_tree(tree)->erase(key);
}

forest::Leaf forest::find_leaf(details::string tree_name, details::tree_t::key_type key)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-KEY_" + nt->get_name() + "_" + key);

	details::tree_t::iterator t = nt->find(key);
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}

forest::Leaf forest::find_leaf(details::string tree_name, LEAF_POSITION position)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-POS_" + nt->get_name() + "_" + details::to_string((int)position));

	details::tree_t::iterator t;
	if(position == LEAF_POSITION::BEGIN){
		t = nt->get_tree()->begin();
	} else if(position == LEAF_POSITION::END) {
		t = --nt->get_tree()->end();
	} else {
		throw TreeException(TreeException::ERRORS::BAD_INPUT_PARAMETER);
	}
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}

forest::Leaf forest::find_leaf(details::string tree_name, details::tree_t::key_type key, LEAF_POSITION position)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_owner_ptr tree = find_tree(tree_name);
	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-BNT_" + nt->get_name() + "_" + key + "_" + details::to_string((int)position));

	details::tree_t::iterator t;
	if(position == LEAF_POSITION::LOWER){
		t = nt->get_tree()->lower_bound(key);
	} else if(position == LEAF_POSITION::UPPER) {
		t = nt->get_tree()->upper_bound(key);
	} else {
		throw TreeException(TreeException::ERRORS::BAD_INPUT_PARAMETER);
	}
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}


forest::Leaf forest::find_leaf(Tree tree, details::tree_t::key_type key)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-KEY_" + nt->get_name() + "_" + key);

	details::tree_t::iterator t = nt->find(key);
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}

forest::Leaf forest::find_leaf(Tree tree, LEAF_POSITION position)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-POS_" + nt->get_name() + "_" + details::to_string((int)position));

	details::tree_t::iterator t;
	if(position == LEAF_POSITION::BEGIN){
		t = nt->get_tree()->begin();
	} else if(position == LEAF_POSITION::END) {
		t = --nt->get_tree()->end();
	} else {
		throw TreeException(TreeException::ERRORS::BAD_INPUT_PARAMETER);
	}
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}

forest::Leaf forest::find_leaf(Tree tree, details::tree_t::key_type key, LEAF_POSITION position)
{
	if(!blooms()){
		throw TreeException(TreeException::ERRORS::FOREST_FOLDED);
	}

	details::tree_ptr nt = details::extract_native_tree(tree);

	L_PUB("[forest::find_leaf]-BNT_" + nt->get_name() + "_" + key + "_" + details::to_string((int)position));

	details::tree_t::iterator t;
	if(position == LEAF_POSITION::LOWER){
		t = nt->get_tree()->lower_bound(key);
	} else if(position == LEAF_POSITION::UPPER) {
		t = nt->get_tree()->upper_bound(key);
	} else {
		throw TreeException(TreeException::ERRORS::BAD_INPUT_PARAMETER);
	}
	details::LeafRecord_ptr rc = details::LeafRecord_ptr(new details::LeafRecord(t, nt));

	return rc;
}

forest::DetachedLeaf forest::make_leaf(details::string data)
{
	return details::detached_leaf_ptr(new details::detached_leaf(details::leaf_value(data)));
}

forest::DetachedLeaf forest::make_leaf(char* buffer, details::uint_t length)
{
	return details::detached_leaf_ptr(new details::detached_leaf(details::file_data_ptr(new details::file_data_t(buffer, length))));
}

forest::DetachedLeaf forest::make_leaf(LeafFile file, details::uint_t start, details::uint_t length)
{
	return details::detached_leaf_ptr(new details::detached_leaf(details::file_data_ptr(new details::file_data_t(file, start, length))));
}

forest::LeafFile forest::create_leaf_file()
{
	details::file_ptr f(DBFS::create());
	f->on_close([](DBFS::File* file){ forest::details::savior->remove_file_async(file->name()); });
	return f;
}


/* Configurations */

void forest::config_root_factor(int root_factor)
{
	details::ROOT_FACTOR = root_factor;
}

void forest::config_default_factor(int default_factor)
{
	details::DEFAULT_FACTOR = default_factor;
}

void forest::config_root_tree(details::string root_tree)
{
	details::ROOT_TREE = root_tree;
}

void forest::config_intr_cache_length(int length)
{
	details::cache::set_intr_cache_length(length);
}

void forest::config_leaf_cache_length(int length)
{
	details::cache::set_leaf_cache_length(length);
}

void forest::config_tree_cache_length(int length)
{
	details::cache::set_tree_cache_length(length);
}

void forest::config_cache_bytes(int bytes)
{
	details::CACHE_BYTES = bytes;
}

void forest::config_chunk_bytes(int bytes)
{
	details::CHUNK_SIZE = bytes;
}

void forest::config_opened_files_limit(int count)
{
	details::OPENED_FILES_LIMIT = count;
}

void forest::config_save_schedule_mks(int mks)
{
	details::SCHEDULE_TIMER = mks;
}

void forest::config_savior_queue_size(int length)
{
	details::SAVIOUR_QUEUE_LENGTH = length;
}

/*********************************************************************************/


void forest::details::create_root_file()
{
	Tree::seed(TREE_TYPES::KEY_STRING, ROOT_TREE, ROOT_FACTOR);
}

void forest::details::open_root()
{
	FOREST = tree_ptr(new Tree(ROOT_TREE));
}

void forest::details::close_root()
{
	FOREST = nullptr;
}

void forest::details::init_savior()
{
	savior = new Savior();
}

void forest::details::release_savior()
{
	delete savior;
}

forest::details::tree_ptr forest::details::reach_tree(string path)
{
	cache::tree_lock();
	tree_ptr t = get_tree(path);
	t->tree_reserve();
	cache::tree_cache_push(t);
	cache::tree_unlock();
	t->ready();
	return t;
}

void forest::details::leave_tree(tree_ptr t)
{
	cache::tree_lock();
	t->tree_release();
	cache::tree_unlock();
}

void forest::details::insert_tree(string name, string file_name, tree_ptr tree)
{
	auto* cache_obj = new cache::tree_cache_ref_t{tree,1};
	cache::tree_lock();
	cache::tree_cache_push(tree);
	cache::tree_cache_r[file_name] = cache_obj;
	tree->get_cached().ref = cache_obj;
	cache::tree_unlock();
	
	savior->put(file_name, SAVE_TYPES::BASE, tree);
	
	file_data_ptr tmp = file_data_ptr(new file_data_t(file_name.c_str(), file_name.size()));
	FOREST->insert(name, std::move(tmp));
	
	leave_tree(tree);
}

void forest::details::erase_tree(string path)
{	
	details::tree_ptr nt = reach_tree(path);
	
	nt->get_tree()->lock_write();
	savior->remove(path, SAVE_TYPES::BASE, nt);
	nt->get_tree()->unlock_write();
	
	nt->get_tree()->clear();

	// Clear cache
	cache::tree_lock();
	cache::tree_cache_remove(nt);
	cache::tree_unlock();
	
	leave_tree(nt);
}

forest::details::tree_ptr forest::details::get_tree(string path)
{
	return Tree::get(path);
}
