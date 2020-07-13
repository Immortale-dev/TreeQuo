#ifndef DBDRIVER_H
#define DBDRIVER_H

#include <functional>
#include <string>
#include "bplustree.hpp"

namespace forest{

	template<typename T, typename V>
	struct DBDriver{
		using bpt_t = BPlusTree<T, V, DBDriver<T,V>>;
		using fn_p = std::function<void(typename bpt_t::node_ptr, typename bpt_t::PROCESS_TYPE, bpt_t*)>;
		using fn = std::function<void(typename bpt_t::node_ptr, bpt_t*)>;
		using fn_move = std::function<void(typename bpt_t::child_item_type_ptr&, int, bpt_t*)>;
		using fn_item = std::function<void(typename bpt_t::child_item_type_ptr&, typename bpt_t::PROCESS_TYPE, bpt_t*)>;
		using fn_item_move = std::function<void(typename bpt_t::node_ptr, bool, bpt_t*)>;
		using fn_n_i = std::function<void(typename bpt_t::node_ptr, typename bpt_t::child_item_type_ptr, bpt_t*)>;
		using fn_n_n = std::function<void(typename bpt_t::node_ptr, typename bpt_t::node_ptr, bpt_t*)>;
		using fn_n_n_n = std::function<void(typename bpt_t::node_ptr, typename bpt_t::node_ptr, typename bpt_t::node_ptr, bpt_t*)>;
		using fn_n_n_r = std::function<void(typename bpt_t::node_ptr, typename bpt_t::node_ptr, typename bpt_t::LEAF_REF, bpt_t*)>;
		
		fn_p enter, leave;
		fn insert, remove;
		fn_move beforeMove, afterMove;
		fn_item itemReserve, itemRelease;
		fn_item_move itemMove;
		fn_p leafReserve, leafRelease;
		fn_n_i leafInsertItem;
		fn_n_i leafDeleteItem;
		fn_n_n_n leafSplit, leafJoin;
		fn_n_n leafShift;
		fn leafLock;
		fn leafFree;
		fn_n_n_r leafRef;
		fn save_base;
		
		DBDriver(
			fn_p enter, 
			fn_p leave, 
			fn insert, 
			fn remove, 
			fn_move beforeMove, 
			fn_move afterMove, 
			fn_item itemReserve, 
			fn_item itemRelease, 
			fn_item_move itemMove, 
			fn_p leafReserve,
			fn_p leafRelease,
			fn_n_i leafInsertItem,
			fn_n_i leafDeleteItem,
			fn_n_n_n leafSplit,
			fn_n_n_n leafJoin,
			fn_n_n leafShift,
			fn leafLock,
			fn leafFree,
			fn_n_n_r leafRef,
			fn save_base
		);
		~DBDriver();
	};

}

template<typename T, typename V>
forest::DBDriver<T,V>::DBDriver(
	fn_p enter, 
	fn_p leave, 
	fn insert, 
	fn remove, 
	fn_move beforeMove, 
	fn_move afterMove, 
	fn_item itemReserve, 
	fn_item itemRelease, 
	fn_item_move itemMove, 
	fn_p leafReserve,
	fn_p leafRelease,
	fn_n_i leafInsertItem,
	fn_n_i leafDeleteItem,
	fn_n_n_n leafSplit,
	fn_n_n_n leafJoin,
	fn_n_n leafShift,
	fn leafLock,
	fn leafFree,
	fn_n_n_r leafRef,
	fn save_base
) : 
	enter(enter), 
	leave(leave), 
	insert(insert), 
	remove(remove), 
	beforeMove(beforeMove), 
	afterMove(afterMove), 
	itemReserve(itemReserve), 
	itemRelease(itemRelease), 
	itemMove(itemMove), 
	leafReserve(leafReserve),
	leafRelease(leafRelease),
	leafInsertItem(leafInsertItem),
	leafDeleteItem(leafDeleteItem),
	leafSplit(leafSplit),
	leafJoin(leafJoin),
	leafShift(leafShift),
	leafLock(leafLock),
	leafFree(leafFree),
	leafRef(leafRef),
	save_base(save_base)
{

}

template<typename T, typename V>
forest::DBDriver<T,V>::~DBDriver()
{
    
}


#endif //DBDRIVER_H
