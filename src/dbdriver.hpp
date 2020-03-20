#ifndef DBDRIVER_H
#define DBDRIVER_H

#include <functional>
#include <string>
#include "bplustree.hpp"

namespace forest{

	template<typename T, typename V>
	struct DBDriver{
		using bpt_t = BPlusTree<T, V, DBDriver<T,V>>;
		using fn_p = std::function<void(typename bpt_t::node_ptr&, typename bpt_t::PROCESS_TYPE, bpt_t*)>;
		using fn = std::function<void(typename bpt_t::node_ptr&, bpt_t*)>;
		using fn_move = std::function<void(typename bpt_t::child_item_type_ptr&, int, bpt_t*)>;
		using fn_item = std::function<void(typename bpt_t::child_item_type_ptr&, typename bpt_t::PROCESS_TYPE, bpt_t*)>;
		fn_p enter, leave;
		fn insert, remove, reserve, release;
		fn_move beforeMove, afterMove;
		fn_item itemReserve, itemRelease;
		fn save_base;
		DBDriver(fn_p enter, fn_p leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove, fn_item itemReserve, fn_item itemRelease, fn save_base);
		~DBDriver();
	};

}

template<typename T, typename V>
forest::DBDriver<T,V>::DBDriver(fn_p enter, fn_p leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove, fn_item itemReserve, fn_item itemRelease, fn save_base) : 
enter(enter), leave(leave), insert(insert), remove(remove), reserve(reserve), release(release), beforeMove(beforeMove), afterMove(afterMove), itemReserve(itemReserve), itemRelease(itemRelease), save_base(save_base)
{

}

template<typename T, typename V>
forest::DBDriver<T,V>::~DBDriver()
{
    
}


#endif //DBDRIVER_H
