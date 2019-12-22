#ifndef DB_H
#define DB_H

#include <string>
#include <exception>
#include "bplustree.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"

class DB{
	
	using string = std::string;
	
	public:
		DB();
		~DB();
		DB(string path);
		
		create_qtree();
		delete_qtree();
		
		insert_qleaf();
		erase_qleaf();
		
		find_qleaf();
		find_qbranch();
		
		void bloom(string path);
		void fold();
	private:
		bplustree* FOREST;
		bool blossomed = false;
		
		void create_root_file();
	
		string ROOT_TREE = "_root";
};

#endif // DB_H
