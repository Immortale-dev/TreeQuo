
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
		
		forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test");
		
		
		forest::insert_leaf("test", "leaf_1", forest::make_leaf("Woohoo!!! Its fucking works!!!"));
		for(int i=0;i<100;i++){
			forest::insert_leaf("test", "test_"+to_string(i), forest::make_leaf("Yes it is! jst for test and number - "+to_string(i)));
		}
		
		for(int i=70;i>=10;i--){
			forest::remove_leaf("test", "test_"+to_string(i));
		}
		
		
		forest::cut_tree("test");
		forest::fold();
	}
	cout << "END" << endl;
}
