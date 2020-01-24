
#include "db.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
using namespace std;

#define DEBUG


void test_files()
{
    atomic<int> ind;
    mutex mtx;
    ind = 0;
    bool br = false;
    vector<thread> v;
    while(!br){
        for(int i=0;i<50;i++){
            if(ind++ >= 5000){
                br = true;
                break;
            }
            thread t([&mtx](int ind){
                //lock_guard<mutex> lock(mtx);
                ofstream f("tmp/mtest/test/t_" + to_string(ind));
                f << "test test test test test test test test test test test test ";
                f.close();
            }, ind.load());
            v.push_back(move(t));
        }
    }
    cout << "SIZE: " << v.size() << endl;
    for(auto &it : v){
        it.join();
    }
}


int main(){
    
    cout << "WTF???" << endl;

    auto t1 = chrono::system_clock::now();
    test_files();
    auto t2 = chrono::system_clock::now();
    
    auto d = chrono::duration_cast<chrono::milliseconds>(t2-t1);
    
    cout << d.count() << endl;
	
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
