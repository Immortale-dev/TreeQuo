#ifndef FOREST_BPLUSTREE_H
#define FOREST_BPLUSTREE_H

#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "BPlusTreeBase.hpp"

namespace forest{
namespace details{

	template <class Key, class T, class P>
	class BPTInternal : public BPlusTreeBaseInternalNode<Key, T>{
		public:
			P data;
			using BPlusTreeBaseInternalNode<Key, T>::BPlusTreeBaseInternalNode;
			~BPTInternal();
	};

	template <class Key, class T, class P>
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
			
			P data;
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
	
	template <class Key, class T, class P>
	BPTInternal<Key, T, P>::~BPTInternal()
	{
		if(!data.is_original){
			this->set_keys(nullptr);
			this->set_nodes(nullptr);
		}
	}

	template <class Key, class T>
	BPTIterator<Key, T>& BPTIterator<Key, T>::operator++()
	{
		return dynamic_cast<BPTIterator<Key, T>& >(BPlusTreeBaseIterator<Key, T>::operator++());
	}

	template <class Key, class T>
	BPTIterator<Key, T>& BPTIterator<Key, T>::operator--()
	{
		return dynamic_cast<BPTIterator<Key, T>& >(BPlusTreeBaseIterator<Key, T>::operator--());
	}

	template <class Key, class T, class P>
	BPTLeaf<Key, T, P>::~BPTLeaf()
	{
		if(data.f){
			data.f->close();
		}
		if(!data.is_original){
			this->set_childs(nullptr);
			set_prev_leaf(nullptr);
			set_next_leaf(nullptr);
		}
	}

	template <class Key, class T, class P>
	void BPTLeaf<Key, T, P>::set_prev_leaf(node_ptr node)
	{
		p_prev_leaf = node;
	}

	template <class Key, class T, class P>
	void BPTLeaf<Key, T, P>::set_next_leaf(node_ptr node)
	{
		p_next_leaf = node;
	}

	template <class Key, class T, class P>
	typename BPTLeaf<Key, T, P>::node_ptr BPTLeaf<Key, T, P>::prev_leaf()
	{
		return p_prev_leaf;
	}

	template <class Key, class T, class P>
	typename BPTLeaf<Key, T, P>::node_ptr BPTLeaf<Key, T, P>::next_leaf()
	{
		return p_next_leaf;
	}


	template <class Key, class T, typename D, class P>
	class BPlusTree : public BPlusTreeBase<Key, T, BPTInternal<Key, T, P>, BPTLeaf<Key, T, P>, BPTIterator<Key, T> > {
		
		public:
			typedef BPlusTreeBase<Key, T, BPTInternal<Key, T, P>, BPTLeaf<Key, T, P>, BPTIterator<Key, T> > Base;
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
			void processOffsetLeafReserve(node_ptr node, int offset);
			void processOffsetLeafRelease(node_ptr node, int offset);
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

	template <class Key, class T, typename D, class P>
	BPlusTree<Key, T, D, P>::BPlusTree(int factor, node_ptr node, long count, D* driver) : BPlusTreeBase<Key, T, BPTInternal<Key, T, P>, BPTLeaf<Key, T, P>, BPTIterator<Key, T> >(factor), driver(driver) 
	{
		init(node);
		this->v_count = count;
	}

	template <class Key, class T, typename D, class P>
	BPlusTree<Key, T, D, P>::~BPlusTree() 
	{
		//delete driver;
	}

	template <class Key, class T, typename D, class P>
	typename BPlusTree<Key, T, D, P>::node_ptr BPlusTree<Key, T, D, P>::get_root_pub()
	{
		return this->get_root();
	}

	template <class Key, class T, typename D, class P>
	typename BPlusTree<Key, T, D, P>::node_ptr BPlusTree<Key, T, D, P>::get_stem_pub()
	{
		return this->get_stem();
	}

	template <class Key, class T, typename D, class P>
	bool BPlusTree<Key, T, D, P>::is_stem_pub(node_ptr node)
	{
		return this->is_stem(node);
	}

	template <class Key, class T, typename D, class P>
	int BPlusTree<Key, T, D, P>::get_factor()
	{
		return this->factor;
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::init(node_ptr node)
	{
		if(this->root){
			this->release_node(this->root);
		}
		this->set_root(node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processSearchNodeStart(node_ptr node, PROCESS_TYPE type)
	{	
		driver->d_enter(node, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processSearchNodeEnd(node_ptr node, PROCESS_TYPE type)
	{	
		driver->d_leave(node, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processInsertNode(node_ptr node)
	{
		driver->d_insert(node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processDeleteNode(node_ptr node)
	{
		driver->d_remove(node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processIteratorNodeReserved(node_ptr node)
	{
		//driver->reserve(node, this);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processIteratorNodeReleased(node_ptr node)
	{
		//driver->release(node, this);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processIteratorMoveStart(childs_type_iterator item, int step)
	{
		driver->d_before_move(item, step);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processIteratorMoveEnd(childs_type_iterator item, int step)
	{
		driver->d_after_move(item, step);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processItemReserve(child_item_type_ptr item, PROCESS_TYPE type)
	{
		driver->d_item_reserve(item, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processItemRelease(child_item_type_ptr item, PROCESS_TYPE type)
	{
		driver->d_item_release(item, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processItemMove(node_ptr node, child_item_type_ptr item)
	{
		driver->d_item_move(node, item);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processOffsetLeafReserve(node_ptr node, int offset)
	{
		driver->d_offset_reserve(node, offset);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processOffsetLeafRelease(node_ptr node, int offset)
	{
		driver->d_offset_release(node, offset);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafReserve(node_ptr node, PROCESS_TYPE type)
	{
		driver->d_reserve(node, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafRelease(node_ptr node, PROCESS_TYPE type)
	{
		driver->d_release(node, type);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafInsertItem(node_ptr node, child_item_type_ptr item)
	{
		driver->d_leaf_insert(node, item);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafDeleteItem(node_ptr node, child_item_type_ptr item)
	{
		driver->d_leaf_delete(node, item);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafSplit(node_ptr node, node_ptr new_node, node_ptr link_node)
	{
		driver->d_leaf_split(node, new_node, link_node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafJoin(node_ptr node, node_ptr join_node, node_ptr link_node)
	{
		driver->d_leaf_join(node, join_node, link_node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafShift(node_ptr node, node_ptr shift_node)
	{
		driver->d_leaf_shift(node, shift_node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafLock(node_ptr node)
	{
		driver->d_leaf_lock(node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafFree(node_ptr node)
	{
		driver->d_leaf_free(node);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::processLeafRef(node_ptr node, node_ptr ref_node, LEAF_REF ref)
	{
		driver->d_leaf_ref(node, ref_node, ref);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::save_base()
	{
		lock_write();
		node_ptr stem = this->get_stem();
		driver->d_save_base(stem);
		unlock_write();
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::lock_read()
	{
		node_ptr stem = this->get_stem();
		processSearchNodeStart(stem, PROCESS_TYPE::READ);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::lock_write()
	{
		node_ptr stem = this->get_stem();
		processSearchNodeStart(stem, PROCESS_TYPE::WRITE);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::unlock_read()
	{
		node_ptr stem = this->get_stem();
		processSearchNodeEnd(stem, PROCESS_TYPE::READ);
	}

	template <class Key, class T, typename D, class P>
	void BPlusTree<Key, T, D, P>::unlock_write()
	{
		node_ptr stem = this->get_stem();
		processSearchNodeEnd(stem, PROCESS_TYPE::WRITE);
	}

} // details
} // forest

#endif // FOREST_BPLUSTREE_H
