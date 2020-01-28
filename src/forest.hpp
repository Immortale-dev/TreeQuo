#ifndef FOREST_H
#define FOREST_H

#include "dbfs.hpp"
#include "listcache.hpp"
#include "dbexception.hpp"
#include "dbutils.hpp"
#include "tree.hpp"

namespace forest{
	
	void create_tree(TREE_TYPES type, string name);
	void delete_tree(string name);
	Tree* find_tree(string name);
	Tree* open_tree(string path);
	void close_tree(string path);
	
	//void insert_leaf(string name);
	//void insert_leaf(tree_t tree);
	//void erase_leaf(string name);
	//void erase_leaf(tree_t tree);
	//find_leaf();
			
	void bloom(string path);
	void fold();
	
	//////////// Private? ////////////
	
	Tree* FOREST;
	bool blossomed = false;
	
	// Root methods
	void open_root();
	void close_root();
	
}

#endif //FOREST_H
