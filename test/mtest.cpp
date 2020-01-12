
#include "db.h"
#include <iostream>
using namespace std;

#define DEBUG


int main(){
	
	DB db;

	db.bloom("tmp/mtest");

	cout << "START" << endl;

	db.create_tree(TREE_TYPES::KEY_INT, "test_int0");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int1");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int2");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int3");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int4");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int5");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int6");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int7");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int8");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int9");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int10");
	db.create_tree(TREE_TYPES::KEY_INT, "test_int11");
	db.delete_tree("test_int0");
	db.delete_tree("test_int1");
	db.delete_tree("test_int2");
	db.delete_tree("test_int3");
	db.delete_tree("test_int4");
	db.delete_tree("test_int5");
	db.delete_tree("test_int6");
	db.delete_tree("test_int7");
	db.delete_tree("test_int8");
	db.delete_tree("test_int9");
	db.delete_tree("test_int10");
	db.delete_tree("test_int11");

	cout << "END" << endl;

	cout << "WOOHOOO" << endl;
}
