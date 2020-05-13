#ifndef FOREST_LOCK
#define FOREST_LOCK

#include "dbutils.hpp"

namespace forest{

	// General Lock for Node
	void lock_read(tree_t::node_ptr node);
	void lock_read(tree_t::Node* node);
	void unlock_read(tree_t::node_ptr node);
	void unlock_read(tree_t::Node* node);
	void lock_write(tree_t::node_ptr node);
	void lock_write(tree_t::Node* node);
	void unlock_write(tree_t::node_ptr node);
	void unlock_write(tree_t::Node* node);
	void lock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type);
	void lock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type);
	bool is_write_locked(tree_t::node_ptr node);
	
	// General Lock for Item
	void lock_read(tree_t::child_item_type_ptr item);
	void unlock_read(tree_t::child_item_type_ptr item);
	void lock_write(tree_t::child_item_type_ptr item);
	void unlock_write(tree_t::child_item_type_ptr item);
	void lock_type(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::child_item_type_ptr item, tree_t::PROCESS_TYPE type);
	
	// Own Lock for Node
	void own_lock(tree_t::node_ptr node);
	void own_unlock(tree_t::node_ptr node);
	int own_inc(tree_t::node_ptr node);
	int own_dec(tree_t::node_ptr node);
	
	// Change Lock for Node
	void change_lock_read(tree_t::node_ptr node);
	void change_lock_read(tree_t::Node* node);
	void change_unlock_read(tree_t::node_ptr node);
	void change_unlock_read(tree_t::Node* node);
	void change_lock_write(tree_t::node_ptr node);
	void change_lock_write(tree_t::Node* node);
	void change_unlock_write(tree_t::node_ptr node);
	void change_unlock_write(tree_t::Node* node);
	void change_lock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type);
	void change_unlock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type);
}

#endif // FOREST_LOCK
