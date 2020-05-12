#include "lock.hpp"

void forest::lock_read(tree_t::node_ptr node)
{
	lock_read(node.get());
}

void forest::lock_read(tree_t::Node* node)
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

void forest::unlock_read(tree_t::node_ptr node)
{
	unlock_read(node.get());
}

void forest::unlock_read(tree_t::Node* node)
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

void forest::lock_write(tree_t::node_ptr node)
{
	lock_write(node.get());
}

void forest::lock_write(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.lock();
	//return;
	
	tl.g.lock();
	tl.wlock = true;
}

void forest::unlock_write(tree_t::node_ptr node)
{
	unlock_write(node.get());
}

void forest::unlock_write(tree_t::Node* node)
{
	auto& tl = node->data.travel_locks;
	
	//tl.m.unlock();
	//return;
	
	tl.wlock = false;
	tl.g.unlock();
}

void forest::lock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type)
{
	lock_type(node.get(), type);
}

void forest::lock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? lock_write(node) : lock_read(node);
}

void forest::unlock_type(tree_t::node_ptr node, tree_t::PROCESS_TYPE type)
{
	unlock_type(node.get(), type);
}

void forest::unlock_type(tree_t::Node* node, tree_t::PROCESS_TYPE type)
{
	(type == tree_t::PROCESS_TYPE::WRITE) ? unlock_write(node) : unlock_read(node);
}

bool forest::is_write_locked(tree_t::node_ptr node)
{
	return node->data.travel_locks.wlock;
}


void forest::own_lock(tree_t::node_ptr node)
{
	node->data.owner_locks.m.lock();
}

void forest::own_unlock(tree_t::node_ptr node)
{
	node->data.owner_locks.m.unlock();
}

int forest::own_inc(tree_t::node_ptr node)
{
	return node->data.owner_locks.c++;
}

int forest::own_dec(tree_t::node_ptr node)
{
	assert(node->data.owner_locks.c > 0);
	return --node->data.owner_locks.c;
}
