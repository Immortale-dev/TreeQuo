#ifndef FOREST_LOCK_H
#define FOREST_LOCK_H

#include "dbutils.hpp"

namespace forest{
namespace details{

	// General Lock for Node
	void lock_read(tree_t::node_ptr& node);
	void lock_read(tree_t::Node* node);
	void unlock_read(tree_t::node_ptr& node);
	void unlock_read(tree_t::Node* node);
	void lock_write(tree_t::node_ptr& node);
	void lock_write(tree_t::Node* node);
	void unlock_write(tree_t::node_ptr& node);
	void unlock_write(tree_t::Node* node);
	void lock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
	void lock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type);
	bool is_write_locked(tree_t::node_ptr& node);
	
	// General Lock for Item
	void lock_read(tree_t::child_item_type_ptr& item);
	void unlock_read(tree_t::child_item_type_ptr& item);
	void lock_write(tree_t::child_item_type_ptr& item);
	void unlock_write(tree_t::child_item_type_ptr& item);
	void lock_type(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type);
	void unlock_type(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type);
	
	// Bunch lock
	void change_lock_bunch(tree_t::node_ptr& node, tree_t::node_ptr& c_node, bool w_prior=false);
	void change_lock_bunch(tree_t::node_ptr& node, tree_t::node_ptr& m_node, tree_t::node_ptr& c_node, bool w_prior=false);
	void change_lock_bunch(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item, bool w_prior=false);
	
	// Own Lock for Node
	void own_lock(tree_t::node_ptr& node);
	void own_unlock(tree_t::node_ptr& node);
	int own_inc(tree_t::node_ptr& node);
	int own_dec(tree_t::node_ptr& node);
	
	// Change Lock for Node
	void change_lock_read(tree_t::node_ptr& node);
	void change_lock_read(tree_t::Node* node);
	void change_unlock_read(tree_t::node_ptr& node);
	void change_unlock_read(tree_t::Node* node);
	void change_lock_write(tree_t::node_ptr& node);
	void change_lock_write(tree_t::Node* node);
	void change_unlock_write(tree_t::node_ptr& node);
	void change_unlock_write(tree_t::Node* node);
	void change_lock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
	void change_unlock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type);
	void change_lock_promote(tree_t::node_ptr& node);
	void change_lock_promote(tree_t::Node* node);
	
} // details
} // forest



inline void forest::details::lock_read(tree_t::node_ptr& node)
{
	lock_read(node.get());
}

inline void forest::details::lock_read(tree_t::Node* node)
{
	//return;
	auto& tl = get_data(node).travel_locks;
	
	tl.m.lock();
	if(tl.c++ == 0){
		tl.g.lock();
	}
	tl.m.unlock();
}

inline void forest::details::unlock_read(tree_t::node_ptr& node)
{
	unlock_read(node.get());
}

inline void forest::details::unlock_read(tree_t::Node* node)
{
	//return;
	auto& tl = get_data(node).travel_locks;
	
	tl.m.lock();
	if(--tl.c == 0){
		tl.g.unlock();
	}
	tl.m.unlock();
}

inline void forest::details::lock_write(tree_t::node_ptr& node)
{
	lock_write(node.get());
}

inline void forest::details::lock_write(tree_t::Node* node)
{
	//return;
	auto& tl = get_data(node).travel_locks;
	
	tl.g.lock();
	tl.wlock = true;
}

inline void forest::details::unlock_write(tree_t::node_ptr& node)
{
	unlock_write(node.get());
}

inline void forest::details::unlock_write(tree_t::Node* node)
{
	//return;
	auto& tl = get_data(node).travel_locks;
	
	tl.wlock = false;
	tl.g.unlock();
}

inline void forest::details::lock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{
	lock_type(node.get(), type);
}

inline void forest::details::lock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? lock_write(node) : lock_read(node);
}

inline void forest::details::unlock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{
	unlock_type(node.get(), type);
}

inline void forest::details::unlock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? unlock_write(node) : unlock_read(node);
}

inline bool forest::details::is_write_locked(tree_t::node_ptr& node)
{
	return get_data(node).travel_locks.wlock;
}

inline void forest::details::lock_read(tree_t::child_item_type_ptr& item)
{
	auto& it = item->item->second;
	it->g.lock();
	if(it->c++ == 0){
		it->m.lock();
	}
	it->g.unlock();
}

inline void forest::details::unlock_read(tree_t::child_item_type_ptr& item)
{
	auto& it = item->item->second;
	it->g.lock();
	if(--it->c == 0){
		it->m.unlock();
	}
	it->g.unlock();
}

inline void forest::details::lock_write(tree_t::child_item_type_ptr& item)
{
	item->item->second->m.lock();
}

inline void forest::details::unlock_write(tree_t::child_item_type_ptr& item)
{
	item->item->second->m.unlock();
}

inline void forest::details::lock_type(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? lock_write(item) : lock_read(item);
}

inline void forest::details::unlock_type(tree_t::child_item_type_ptr& item, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? unlock_write(item) : unlock_read(item);
}

inline void forest::details::own_lock(tree_t::node_ptr& node)
{
	get_data(node).owner_locks.m.lock();
}

inline void forest::details::own_unlock(tree_t::node_ptr& node)
{
	get_data(node).owner_locks.m.unlock();
}

inline int forest::details::own_inc(tree_t::node_ptr& node)
{
	return get_data(node).owner_locks.c++;
}

inline int forest::details::own_dec(tree_t::node_ptr& node)
{
	ASSERT(get_data(node).owner_locks.c > 0);
	return --get_data(node).owner_locks.c;
}

inline void forest::details::change_lock_write(tree_t::node_ptr& node)
{
	change_lock_write(node.get());
}

inline void forest::details::change_lock_write(tree_t::Node* node)
{
	get_data(node).change_locks.m.lock();
}

inline void forest::details::change_unlock_write(tree_t::node_ptr& node)
{
	change_unlock_write(node.get());
}

inline void forest::details::change_unlock_write(tree_t::Node* node)
{
	get_data(node).change_locks.m.unlock();
}

inline void forest::details::change_lock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? change_lock_write(node) : change_lock_read(node);
}

inline void forest::details::change_unlock_type(tree_t::node_ptr& node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? change_unlock_write(node) : change_unlock_read(node);
}

#endif // FOREST_LOCK_H
