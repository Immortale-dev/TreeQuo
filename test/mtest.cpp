
#include "forest.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
using namespace std;

//#define DEBUG



int main(){
    
    cout << "START" << endl;
    {
		forest::bloom("tmp/mtest");
		
		forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test");
		forest::insert_leaf("test", "leaf_1", forest::file_data_t::from_string("Woohoo!!! Its fucking works!!!"));
		for(int i=0;i<100;i++){
			forest::insert_leaf("test", "test_"+to_string(i), forest::file_data_t::from_string("Yes it is! jst for test and number - "+to_string(i)));
		}
		
		for(int i=70;i>=10;i--){
			forest::erase_leaf("test", "test_"+to_string(i));
		}
		
		forest::file_data_t t = forest::find_leaf("test", "test_80");
		char tmp[30];
		t.reset();
		int count = t.read(tmp,30);
		cout << "LEAF 80: " << string(tmp,count) << endl;
		
		//cout << "------------------DELETE-------------------" << endl;
		
		forest::delete_tree("test");
	
		forest::fold();
	}
	cout << "END" << endl;
	//DB db;

	//db.bloom("tmp/mtest");

	//cout << "START" << endl;

	/*db.create_tree(TREE_TYPES::KEY_INT, "test_int0");
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
	db.delete_tree("test_int11");*/

	//cout << "END" << endl;

	//cout << "WOOHOOO" << endl;
    
  
    
}
