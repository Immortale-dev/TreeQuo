#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "BPlusTreeBase.hpp"

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
		std::condition_variable cond;
		bool shared_lock = false;
	} change_locks;
	std::shared_ptr<void> drive_data;
};

template <class Key, class T, typename D>
class BPlusTree : public BPlusTreeBase<Key, T, node_addition> {
	
	public:
		typedef BPlusTreeBase<Key, T, node_addition> Base;
		typedef Key key_type;
		typedef T val_type;
		typedef typename Base::Node Node;
		typedef typename Base::InternalNode InternalNode;
		typedef typename Base::LeafNode LeafNode;
		typedef typename Base::iterator iterator;
 		typedef std::shared_ptr<Node> node_ptr;
 		typedef typename Node::child_item_type child_item_type;
 		typedef typename Node::child_item_type_ptr child_item_type_ptr;
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
		
		using Base::update_positions;
		
	private:
		void processSearchNodeStart(node_ptr node, PROCESS_TYPE type);
		void processSearchNodeEnd(node_ptr node, PROCESS_TYPE type);
		void processInsertNode(node_ptr node);
		void processDeleteNode(node_ptr node);
		void processIteratorNodeReserved(node_ptr node);
		void processIteratorNodeReleased(node_ptr node);
		void processIteratorMoveStart(child_item_type_ptr item, int step);
		void processIteratorMoveEnd(child_item_type_ptr item, int step);
		void processItemReserve(child_item_type_ptr item, PROCESS_TYPE type);
		void processItemRelease(child_item_type_ptr item, PROCESS_TYPE type);
		void processItemMove(node_ptr node, bool release);
		void processLeafReserve(node_ptr node, PROCESS_TYPE type);
		void processLeafRelease(node_ptr node, PROCESS_TYPE type);
		void processLeafInsertItem(node_ptr node, child_item_type_ptr item);
		void processLeafDeleteItem(node_ptr node, child_item_type_ptr item);
		void processLeafSplit(node_ptr node, node_ptr new_node, node_ptr link_node);
		void processLeafJoin(node_ptr node, node_ptr join_node, node_ptr link_node);
		void processLeafShift(node_ptr node, node_ptr shift_node);
		void processLeafFree(node_ptr node);
		void processLeafRef(node_ptr node, node_ptr ref_node, LEAF_REF ref);
		
		D* driver;
};

template <class Key, class T, typename D>
BPlusTree<Key, T, D>::BPlusTree(int factor, node_ptr node, long count, D* driver) : BPlusTreeBase<Key, T, node_addition>(factor), driver(driver) 
{
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
	//Base::processSearchNodeStart(node);
	driver->enter(node, type, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processSearchNodeEnd(node_ptr node, PROCESS_TYPE type)
{
	driver->leave(node, type, this);
	//Base::processSearchNodeEnd(node);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processInsertNode(node_ptr node)
{
	driver->insert(node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processDeleteNode(node_ptr node)
{
	driver->remove(node, this);
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
void BPlusTree<Key, T, D>::processIteratorMoveStart(child_item_type_ptr item, int step)
{
	driver->beforeMove(item, step, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processIteratorMoveEnd(child_item_type_ptr item, int step)
{
	driver->afterMove(item, step, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemReserve(child_item_type_ptr item, PROCESS_TYPE type)
{
	driver->itemReserve(item, type, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemRelease(child_item_type_ptr item, PROCESS_TYPE type)
{
	driver->itemRelease(item, type, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processItemMove(node_ptr node, bool release)
{
	driver->itemMove(node, release, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafReserve(node_ptr node, PROCESS_TYPE type)
{
	driver->leafReserve(node, type, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafRelease(node_ptr node, PROCESS_TYPE type)
{
	driver->leafRelease(node, type, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafInsertItem(node_ptr node, child_item_type_ptr item)
{
	driver->leafInsertItem(node, item, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafDeleteItem(node_ptr node, child_item_type_ptr item)
{
	driver->leafDeleteItem(node, item, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafSplit(node_ptr node, node_ptr new_node, node_ptr link_node)
{
	driver->leafSplit(node, new_node, link_node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafJoin(node_ptr node, node_ptr join_node, node_ptr link_node)
{
	driver->leafJoin(node, join_node, link_node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafShift(node_ptr node, node_ptr shift_node)
{
	driver->leafShift(node, shift_node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafFree(node_ptr node)
{
	driver->leafFree(node, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::processLeafRef(node_ptr node, node_ptr ref_node, LEAF_REF ref)
{
	driver->leafRef(node, ref_node, ref, this);
}

template <class Key, class T, typename D>
void BPlusTree<Key, T, D>::save_base()
{
	node_ptr stem = this->get_stem();
	processSearchNodeStart(stem, PROCESS_TYPE::WRITE);
	driver->save_base(stem, this);
	processSearchNodeEnd(stem, PROCESS_TYPE::WRITE);
}


#endif //BPLUSTREE_H
