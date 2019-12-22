#ifndef DBDRIVER_H
#define DBDRIVER_H

#include <functional>
#include <string>
#include "bplustree.hpp"

template<typename T>
struct DBDriver{
    using file_pos_t = long long int;
    using bpt_t = BPlusTree<T, file_pos_t, DBDriver>;
    using fn = std::function<void(bpt_t::node_ptr, bpt_t*)>;
    using fn_move = std::function<void(bpt_t::iterator, bpt_t::node_ptr, int step, bpt_t*)>;
    fn enter, leave, insert, remove, reserver, release;
    fn_move beforeMove, afterMove;
    DBDriver(fn enter, fn leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove);
    ~DBDriver();
};

template<typename T>
DBDriver<T>::DBDriver(fn enter, fn leave, fn insert, fn remove, fn reserve, fn release, fn_move beforeMove, fn_move afterMove) : 
enter(enter), leave(leave), insert(insert), remove(remove), reserve(reserve), release(release), beforeMove(beforeMove), afterMove(afterMove)
{

}

template<typename T>
DBDriver<T>::~DBDriver()
{
    
}


#endif //DBDRIVER_H