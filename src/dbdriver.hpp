#ifndef DBDRIVER_H
#define DBDRIVER_H

#include <functional>
#include <string>
#include "bplustree.hpp"

namespace forest{

	template<typename T, typename V>
	struct DBDriver{
		using bpt_t = BPlusTree<T, V, DBDriver<T,V>>;
		using fn = std::function<void(typename bpt_t::node_ptr&, bpt_t*)>;
		using fn_move = std::function<void(typename bpt_t::child_item_type_ptr&, int, bpt_t*)>;
		fn enter, leave, insert, remove, reserve, release;
		fn_move beforeMove, afterMove;
		fn save_base;
		DBDriver(fn enter, fn leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove, fn save_base);
		~DBDriver();
	};

}

template<typename T, typename V>
forest::DBDriver<T,V>::DBDriver(fn enter, fn leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove, fn save_base) : 
enter(enter), leave(leave), insert(insert), remove(remove), reserve(reserve), release(release), beforeMove(beforeMove), afterMove(afterMove), save_base(save_base)
{

}

template<typename T, typename V>
forest::DBDriver<T,V>::~DBDriver()
{
    
}


#endif //DBDRIVER_H
