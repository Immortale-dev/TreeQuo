#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include "BPlusTreeBase.hpp"

extern int hooks_time;

namespace forest{
	struct node_addition{
		struct{
			std::mutex m,g;
			int c = 0;
			bool wlock = false;
		} travel_locks;
		struct{
			std::mutex m;
			int c = 0;
		} owner_locks;
		struct{
			std::mutex m,g,p;
			bool wlock = false;
			int c = 0;
			bool promote = false;
			std::condition_variable cond, p_cond;
			bool shared_lock = false;
		} change_locks;
		std::shared_ptr<void> drive_data;
		std::shared_ptr<DBFS::File> f;
	};
}

template <class Key, class T>
class BPTInternal : public BPlusTreeBaseInternalNode<Key, T>{
	public:
		forest::node_addition data;
		using BPlusTreeBaseInternalNode<Key, T>::BPlusTreeBaseInternalNode;
};

template <class Key, class T>
class BPTLeaf : public BPlusTreeBaseLeafNode<Key, T>{
	public:
		using BPlusTreeBaseLeafNode<Key, T>::BPlusTreeBaseLeafNode;
		~BPTLeaf();
		
		typedef BPlusTreeBaseNode<Key, T> Node;
		typedef std::shared_ptr<Node> node_ptr;
		
		void set_prev_leaf(node_ptr node);
		void set_next_leaf(node_ptr node);
		node_ptr prev_leaf();
		node_ptr next_leaf();
		
		node_ptr p_prev_leaf;
		node_ptr p_next_leaf;
		
		forest::node_addition data;
};

template <class Key, class T>
class BPTIterator : public BPlusTreeBaseIterator<Key, T>{
	public: 
		typedef BPTIterator<Key, T> self_type;
		using BPlusTreeBaseIterator<Key, T>::BPlusTreeBaseIterator;
		using BPlusTreeBaseIterator<Key, T>::get_base;
		self_type& operator++();
		self_type& operator--();
		
};

template <class Key, class T>
BPTIterator<Key, T>& BPTIterator<Key, T>::operator++()
{
	return dynamic_cast<BPTIterator<Key, T>& >(BPlusTreeBaseIterator<Key, T>::operator++());
	//return *this;
}

template <class Key, class T>
BPTIterator<Key, T>& BPTIterator<Key, T>::operator--()
{
	return dynamic_cast<BPTIterator<Key, T>& >(BPlusTreeBaseIterator<Key, T>::operator--());
	//return *this;
}

template <class Key, class T>
BPTLeaf<Key, T>::~BPTLeaf()
{
	if(data.f){
		//std::cout << "DESCRUCT_LEAF: " + data.f->name() + " " + std::to_string(data.f.use_count()) + "\n";
		data.f->close();
	}
}

template <class Key, class T>
void BPTLeaf<Key, T>::set_prev_leaf(node_ptr node)
{
	p_prev_leaf = node;
}

template <class Key, class T>
void BPTLeaf<Key, T>::set_next_leaf(node_ptr node)
{
	p_next_leaf = node;
}

template <class Key, class T>
typename BPTLeaf<Key,T>::node_ptr BPTLeaf<Key,T>::prev_leaf()
{
	return p_prev_leaf;
}

template <class Key, class T>
typename BPTLeaf<Key,T>::node_ptr BPTLeaf<Key,T>::next_leaf()
{
	return p_next_leaf;
}


template <class Key, class T, typename D>
class BPlusTree : public BPlusTreeBase<Key, T, BPTInternal<Key, T>, BPTLeaf<Key, T>, BPTIterator<Key, T> > {
	
	public:
		typedef BPlusTreeBase<Key, T, BPTInternal<Key, T>, BPTLeaf<Key, T>, BPTIterator<Key, T> > Base;
		typedef Key key_type;
		typedef T val_type;
		typedef typename Base::Node Node;
		typedef typename Base::InternalNode InternalNode;
		typedef typename Base::LeafNode LeafNode;
		typedef typename Base::iterator iterator;
 		typedef std::shared_ptr<Node> node_ptr;
 		typedef typename Node::child_item_type child_item_type;
 		typedef typename Node::child_item_type_ptr child_item_type_ptr;
 		typedef typename Node::childs_type_iterator childs_type_iterator;
		typedef typename Base::PROCESS_TYPE PROCESS_TYPE;
		typedef typename Base::LEAF_REF LEAF_REF;
		
		friend D;
		
		BPlusTree(int factor, node_ptr node, long count, D* driver);
		~BPlusTree();
		void init(node_ptr node);
		int get_factor();
		node_ptr get_root_pub();
		node_ptr get_stem_pub();
		bool is_stem_pub(node_ptr node);
		void save_base();
		
		void lock_read();
		void lock_write();
		void unlock_read();
		void unlock_write();
		
		using Base::update_positions;
		using Base::create_entry_item;
		
	private:
		void processSearchNodeStart(node_ptr node, PROCESS_TYPE type);
		void processSearchNodeEnd(node_ptr node, PROCESS_TYPE type);
		void processInsertNode(node_ptr node);
		void processDeleteNode(node_ptr node);
		void processIteratorNodeReserved(node_ptr node);
		void processIteratorNodeReleased(node_ptr node);
		void processIteratorMoveStart(childs_type_iterator item, int step);
		void processIteratorMoveEnd(childs_type_iterator item, int step);
		void processItemReserve(child_item_type_ptr item, PROCESS_TYPE type);
		void processItemRelease(child_item_type_ptr item, PROCESS_TYPE type);
		void processItemMove(node_ptr node, child_item_type_ptr item);
		void processLeafReserve(node_ptr node, PROCESS_TYPE type);
		void processLeafRelease(node_ptr node, PROCESS_TYPE type);
		void processLeafInsertItem(node_ptr node, child_item_type_ptr item);
		void processLeafDeleteItem(node_ptr node, child_item_type_ptr item);
		void processLeafSplit(node_ptr node, node_ptr new_node, node_ptr link_node);
		void processLeafJoin(node_ptr node, node_ptr join_node, node_ptr link_node);
		void processLeafShift(node_ptr node, node_ptr shift_node);
		void processLeafLock(node_ptr node);
		void processLeafFree(node_ptr node);
		void processLeafRef(node_ptr node, node_ptr ref_node, LEAF_REF ref);
		
		D* driver;
};

template <class Key, class T, typename D>
BPlusTree<Key, T, D>::BPlusTree(int factor, node_ptr node, long count, D* driver) : BPlusTreeBase<Key, T, BPTInternal<Key, T>, BPTLeaf<Key, T>, BPTIterator<Key, T> >(factor), driver(driver) 
{
	//std::cout << "BPT_CREATED=========================================================================\n";
	init(node);
	this->v_count = count;
}

template <class Key, class T, typename D>
BPlusTree<Key, T, D>::~BPlusTree() 
{
	delete driver;
}

template <class Key, class T, typename D>
typename BPlusTree<Key, T, D>::node_ptr BPlusTree<Key, T, D>::get_root_pub()
{
	return this->get_root();
}

template <class Key, class T, typename D>
typename BPlusTree<Key, T, D>::node_ptr BPlusTree<Key, T, D>::get_stem_pub()
{
	return this->get_stem();
}

template <class Key, class T, typename D>
bool BPlusTree<Key, T, D>::is_stem_pub(node_ptr node)
{
	return this->is_stem(node);
}

template <class Key, class T, typename D>
int BPlusTree<Key, T, D>::get_factor()
{
	return this->factor;
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::init(node_ptr node)
{
	if(this->root){
		this->release_node(this->root);
	}
	this->set_root(node);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processSearchNodeStart(node_ptr node, PROCESS_TYPE type)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->enter(node, type, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processSearchNodeEnd(node_ptr node, PROCESS_TYPE type)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leave(node, type, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processInsertNode(node_ptr node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->insert(node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processDeleteNode(node_ptr node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->remove(node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processIteratorNodeReserved(node_ptr node)
{
	//driver->reserve(node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processIteratorNodeReleased(node_ptr node)
{
	//driver->release(node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processIteratorMoveStart(childs_type_iterator item, int step)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->beforeMove(item, step, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processIteratorMoveEnd(childs_type_iterator item, int step)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->afterMove(item, step, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemReserve(child_item_type_ptr item, PROCESS_TYPE type)
{
	auto p1 = std::chrono::high_resolution_clock::now();

	driver->itemReserve(item, type, this);
	
	auto p2 = std::chrono::high_resolution_clock::now();
	hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemRelease(child_item_type_ptr item, PROCESS_TYPE type)
{
	auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->itemRelease(item, type, this);
	
	auto p2 = std::chrono::high_resolution_clock::now();
	hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemMove(node_ptr node, child_item_type_ptr item)
{
	//std::cout << "WTDF!!!!!!!!!\n";
	auto p1 = std::chrono::high_resolution_clock::now();

	driver->itemMove(node, item, this);
	
	auto p2 = std::chrono::high_resolution_clock::now();
	hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafReserve(node_ptr node, PROCESS_TYPE type)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafReserve(node, type, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafRelease(node_ptr node, PROCESS_TYPE type)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafRelease(node, type, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafInsertItem(node_ptr node, child_item_type_ptr item)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafInsertItem(node, item, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafDeleteItem(node_ptr node, child_item_type_ptr item)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafDeleteItem(node, item, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafSplit(node_ptr node, node_ptr new_node, node_ptr link_node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafSplit(node, new_node, link_node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafJoin(node_ptr node, node_ptr join_node, node_ptr link_node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->leafJoin(node, join_node, link_node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafShift(node_ptr node, node_ptr shift_node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafShift(node, shift_node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafLock(node_ptr node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafLock(node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafFree(node_ptr node)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	driver->leafFree(node, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafRef(node_ptr node, node_ptr ref_node, LEAF_REF ref)
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();

	driver->leafRef(node, ref_node, ref, this);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::save_base()
{
	/*======*///auto p1 = std::chrono::high_resolution_clock::now();
	
	node_ptr stem = this->get_stem();
	processSearchNodeStart(stem, PROCESS_TYPE::WRITE);
	driver->save_base(stem, this);
	processSearchNodeEnd(stem, PROCESS_TYPE::WRITE);
	
	/*======*///auto p2 = std::chrono::high_resolution_clock::now();
	/*======*///hooks_time += std::chrono::duration_cast<std::chrono::microseconds>(p2-p1).count();
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::lock_read()
{
	node_ptr stem = this->get_stem();
	processSearchNodeStart(stem, PROCESS_TYPE::READ);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::lock_write()
{
	node_ptr stem = this->get_stem();
	processSearchNodeStart(stem, PROCESS_TYPE::WRITE);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::unlock_read()
{
	node_ptr stem = this->get_stem();
	processSearchNodeEnd(stem, PROCESS_TYPE::READ);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::unlock_write()
{
	node_ptr stem = this->get_stem();
	processSearchNodeEnd(stem, PROCESS_TYPE::WRITE);
}

#endif //BPLUSTREE_H
