#ifndef LISTCACHE_H
#define LISTCACHE_H

#include <unordered_map>
#include <utility>
#include <list>
#include <functional>

template<typename Key, typename T>
class ListCache{
	public:
		using item_t = std::pair<Key, T>;
		using list_t = std::list<item_t>;
		using list_t_iterator = typename list_t::iterator;
		using hash_t = std::unordered_map<Key, list_t_iterator>;
		using hash_t_iterator = typename hash_t::iterator;
		using callback_t = std::function<void(Key)>;
		
		ListCache();
		ListCache(int size);
		ListCache(int size, callback_t fn);
		~ListCache();
		
		T& get(Key& key);
		item_t& back();
		item_t& front();
		bool has(Key& key);
		void push(item_t val);
		void push(Key key, T val);
		void remove(Key& key);
		void pop();
		
		void resize(int size);
		void set_callback(callback_t fn);
		void clear();
		int size();
		
	private:
		int max_size = 10;
		bool destructed = false;
		hash_t container;
		list_t list;
		callback_t fn;
		
		void remove_overflow();
		
};

template<typename Key, typename T>
ListCache<Key, T>::ListCache()
{
	
}

template<typename Key, typename T>
ListCache<Key, T>::~ListCache()
{
	destructed = true;
}

template<typename Key, typename T>
ListCache<Key, T>::ListCache(int size)
{
	resize(size);
}

template<typename Key, typename T>
ListCache<Key, T>::ListCache(int size, callback_t fn) : fn(fn)
{
	resize(size);
}

template<typename Key, typename T>
bool ListCache<Key, T>::has(Key& key)
{
	return container.count(key);
}

template<typename Key, typename T>
T& ListCache<Key, T>::get(Key& key)
{
	auto it = container[key];
	list.splice(list.begin(), list, it);
	return it->second;
}

template<typename Key, typename T>
void ListCache<Key, T>::push(item_t val)
{
    if(container.count(val.first)){
		get(val.first);
        return;
    }
	list.push_front(val);
	container[val.first] = list.begin();
	remove_overflow();
}

template<typename Key, typename T>
void ListCache<Key, T>::push(Key key, T val)
{
	push(std::make_pair(key,val));
}

template<typename Key, typename T>
typename ListCache<Key, T>::item_t& ListCache<Key, T>::back()
{
	return list.back();
}

template<typename Key, typename T>
typename ListCache<Key, T>::item_t& ListCache<Key, T>::front()
{
	return list.front();
}

template<typename Key, typename T>
void ListCache<Key, T>::remove(Key& key)
{
	if(!container.count(key))
		return;
	list.erase(container[key]);
	container.erase(key);
}

template<typename Key, typename T>
void ListCache<Key, T>::pop()
{
	Key k = list.back().first;
	container.erase(k);
	list.pop_back();
	if(fn) fn(k);
}

template<typename Key, typename T>
void ListCache<Key, T>::resize(int size)
{
	max_size = size;
	remove_overflow();
}

template<typename Key, typename T>
void ListCache<Key, T>::set_callback(callback_t fn)
{
	this->fn = fn;
}

template<typename Key, typename T>
void ListCache<Key, T>::clear()
{
	while(size()){
		pop();
	}
	//list.clear();
	//container.clear();
}

template<typename Key, typename T>
int ListCache<Key, T>::size()
{
	return list.size();
}


template<typename Key, typename T>
void ListCache<Key, T>::remove_overflow()
{
	while(size() > max_size){
		pop();
	}
}


#endif // LISTCACHE
