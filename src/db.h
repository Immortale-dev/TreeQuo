#ifndef DB_H
#define DB_H

#include <string>
#include "bplustree.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"

class DB{
	
	using string = std::string;
	
	public:
		DB();
		~DB();
		DB(string path);
		void init();
		
	private:
		bool initialized = false;
	
};

#endif // DB_H
