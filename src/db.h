#ifndef DB_H
#define DB_H

#include <string>
#include <exception>
#include "bplustree.hpp"
#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbdriver.hpp"

class DB{
	
	using string = std::string;
	using root_t = BPlusTree<string, int, DBDriver<string>>;
	using int_t = long long int;

	enum class KEY_TYPES { INT, STRING };

	public:
		DB();
		~DB();
		DB(string path);
		
		void create_qtree(KEY_TYPES type, string name);
		void delete_qtree(string name);
		
		void insert_qleaf(string name);
		void erase_qleaf(string name);
		
		//find_qleaf();
		//find_qbranch();
		
		void bloom(string path);
		void fold();
	private:
		root_t* FOREST;
		bool blossomed = false;
		
		void create_root_file();

		template<typename T>
		DBDriver<T> get_driver();
	
		string ROOT_TREE = "_root";
};

#endif // DB_H
