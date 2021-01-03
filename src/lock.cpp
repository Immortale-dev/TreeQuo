#include "lock.hpp"


void forest::details::change_lock_bunch(tree_t::node_ptr& node, tree_t::node_ptr& c_node, bool w_prior)
{
	// quick-access
	auto& ch_node = get_data(node).change_locks;
	auto& ch_shift_node = get_data(c_node).change_locks;
	
	if(w_prior){
		// Priority lock
		ch_node.p.lock();
		ch_shift_node.p.lock();
		/// prior_lock{
		
		// Assign flag
		ch_node.shared_lock = true;
		ch_shift_node.shared_lock = true;
		
		/// }prior_lock
		ch_node.p.unlock();
		ch_shift_node.p.unlock();
	}
	
	// Lock
	std::lock(ch_node.m, ch_shift_node.m);
	
	if(w_prior){
		// Priority lock
		ch_node.p.lock();
		ch_shift_node.p.lock();
		/// prior_lock{
		
		// Cleanup
		ch_node.shared_lock = false;
		ch_shift_node.shared_lock = false;
		
		// Notify threads
		ch_node.cond.notify_all();
		ch_shift_node.cond.notify_all();
		
		/// }prior_lock
		ch_node.p.unlock();
		ch_shift_node.p.unlock();
	}
}

void forest::details::change_lock_bunch(tree_t::node_ptr& node, tree_t::node_ptr& m_node, tree_t::node_ptr& c_node, bool w_prior)
{
	// quick-access
	auto& ch_node = get_data(node).change_locks;
	auto& ch_new_node = get_data(m_node).change_locks;
	auto& ch_link_node = get_data(c_node).change_locks; 
	
	if(w_prior){
		// Priority lock
		ch_node.p.lock();
		ch_new_node.p.lock();
		ch_link_node.p.lock();
		/// prior_lock{
		
		// assing flag
		ch_node.shared_lock = true;
		ch_new_node.shared_lock = true;
		ch_link_node.shared_lock = true;
		
		/// }prior_lock
		ch_node.p.unlock();
		ch_new_node.p.unlock();
		ch_link_node.p.unlock();
	}
	
	// Lock nodes
	std::lock(ch_node.m, ch_new_node.m, ch_link_node.m);
	
	if(w_prior){
		// Priority lock
		ch_node.p.lock();
		ch_new_node.p.lock();
		ch_link_node.p.lock();
		/// prior_lock{
		
		// Cleanup
		ch_node.shared_lock = false;
		ch_new_node.shared_lock = false;
		ch_link_node.shared_lock = false;
		
		// Notify threads
		ch_node.cond.notify_all();
		ch_new_node.cond.notify_all();
		ch_link_node.cond.notify_all();
		
		/// }prior_lock
		ch_node.p.unlock();
		ch_new_node.p.unlock();
		ch_link_node.p.unlock();
	}
}

void forest::details::change_lock_bunch(tree_t::node_ptr& node, tree_t::child_item_type_ptr& item, bool w_prior)
{	
	// Quick-access
	auto& ch_node = get_data(node).change_locks;
	
	// Set the flags
	if(w_prior){
		ch_node.p.lock();
		/// prior_lock{
		ch_node.shared_lock = true;
		item->item->second->shared_lock = true;
		/// }prior_lock
		ch_node.p.unlock();
	}
	
	// Lock
	std::lock(item->item->second->m, ch_node.m);
	
	// Notify
	if(w_prior){
		ch_node.p.lock();
		/// prior_lock{
		ch_node.shared_lock = false;
		item->item->second->shared_lock = false;
		ch_node.cond.notify_all();
		/// }prior_lock
		ch_node.p.unlock();
	}
}

void forest::details::change_lock_read(tree_t::node_ptr& node)
{
	change_lock_read(node.get());
}

void forest::details::change_lock_read(tree_t::Node* node)
{
	auto& ch_node = get_data(node).change_locks;
	std::unique_lock<std::mutex> lock(ch_node.g);
	while(ch_node.promote){
		ch_node.p_cond.wait(lock);
	}
	if(ch_node.c++ == 0){
		ch_node.m.lock();
	}
}

void forest::details::change_unlock_read(tree_t::node_ptr& node)
{
	change_unlock_read(node.get());
}

void forest::details::change_unlock_read(tree_t::Node* node)
{
	auto& ch_node = get_data(node).change_locks;
	ch_node.g.lock();
	if(--ch_node.c == 0){
		ch_node.m.unlock();
	}
	// Notify if there is lock promotion
	if(ch_node.promote){
		ch_node.cond.notify_all();
	}
	ch_node.g.unlock();
}

void forest::details::change_lock_promote(tree_t::node_ptr& node)
{
	change_lock_promote(node.get());
}

void forest::details::change_lock_promote(tree_t::Node* node)
{
	auto& ch_node = get_data(node).change_locks;
	std::unique_lock<std::mutex> lock(ch_node.g);
	
	ASSERT(!ch_node.promote);
	
	ch_node.promote = true;
	while(ch_node.c > 1){
		ch_node.p_cond.wait(lock);
	}
	
	ASSERT(ch_node.c == 1);
	
	--ch_node.c;
	ch_node.promote = false;
	ch_node.p_cond.notify_all();
}
