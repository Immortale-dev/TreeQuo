#include <iostream>
#include <thread>
#include <queue>
#include <unordered_set>
#include <unordered_map>


#define DEBUG
//#define TEST_ONLY_RULE

#include "qtest.hpp"
using namespace std;
#include "forest.hpp"
#include "src/helper.cpp"

SCENARIO_START

DESCRIBE("[RC]", {
    
    BEFORE_ALL({
		config_low();
        forest::bloom("tmp/t3");
    });
    
    AFTER_ALL({
        forest::fold();
        // Remove dirs?
    });
    
    DESCRIBE("Do 400 operations in 400 threads", {
        int tests_count = 400;
        
        vector<vector<string> > tree_keys_check;
        vector<thread> threads;
        
        unordered_set<string> should_contains;
        queue<int> q;
        
        vector<string> complete;
        
        bool num_assert = true;
        
        BEFORE_ALL({
            
            forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test", 10);
            
            unordered_set<int> check;
            
            for(int i=0;i<300;i++){
                int rnd = rand()%10000 + 200;
                if(!check.count(rnd)){
                    check.insert(rnd);
                    forest::insert_leaf("test", "p"+std::to_string(rnd), forest::make_leaf("val_" + std::to_string(rnd*rnd)));
                }
            }
            
            for(auto& it : check){
                q.push(it);
                should_contains.insert("p"+std::to_string(it));
            }
            mutex m;
            
            for(int i=0;i<tests_count;i++){
                int rnd = rand()%10000 + 11200;
                thread t([&m, &q, &check, &tree_keys_check, &should_contains, &complete, &num_assert](int index, int rnd){
                    int cs = index%4;
                    if(cs == 1){
						//return;
                        auto it = forest::find_leaf("test", forest::LEAF_POSITION::BEGIN);
                        vector<string> keys;
                        do{
                            string key = it->key();
                            keys.push_back(key);
                            auto val = it->val();
                            string sval = read_leaf(val);
                            
                            if(sval.substr(0,3) == "new"){
                                continue;
                            }
                            int k=0, v=0;
                            try{
                                k = std::stoi(key.substr(1));
                                v = std::stoi(sval.substr(4));
                            } catch (std::invalid_argument const &e) {
                                std::cout << "K: " + key + " " + sval + "\n";
                                assert(false);
                            }
                            if(k*k != v){
                                num_assert = false;
                            }
                        }while(it->move_forward());
                        lock_guard<mutex> lock(m);
                        tree_keys_check.push_back(keys);
                        complete.push_back("move");
                    } else if (cs == 2){
						//return;
                        auto it = forest::find_leaf("test", forest::LEAF_POSITION::END);
                        vector<string> keys;
                        do{
                            string key = it->key();
                            keys.push_back(key);
                            auto val = it->val();
                            string sval = read_leaf(val);
                            
                            if(sval.substr(0,3) == "new"){
                                continue;
                            }
                            int k=0, v=0;
                            try{
                                k = std::stoi(key.substr(1));
                                v = std::stoi(sval.substr(4));
                            } catch (std::invalid_argument const &e) {
                                std::cout << "K: " + key + " " + sval + "\n";
                                assert(false);
                            }
                            if(k*k != v){
                                num_assert = false;
                            }
                        }while(it->move_back());
                        lock_guard<mutex> lock(m);
                        tree_keys_check.push_back(keys);
                        complete.push_back("move");
                    } else if(cs == 3) {
						//return;
                        int p;
                        {
                            lock_guard<mutex> lock(m);
                            p = q.front();
                            q.pop();
                        }
                        forest::remove_leaf("test", "p"+std::to_string(p));
                        lock_guard<mutex> lock(m);
                        should_contains.erase("p"+std::to_string(p));
                        complete.push_back("erase");
                    } else {
                        {
                            lock_guard<mutex> lock(m);
                            if(check.count(rnd)){
                                return;
                            }
                            check.insert(rnd);
                        }
                        forest::insert_leaf("test", "p"+std::to_string(rnd), forest::make_leaf("val_" + std::to_string(rnd*rnd)));
                        lock_guard<mutex> lock(m);
                        complete.push_back("insert");
                    }
                }, i, rnd);
                threads.push_back(move(t));
            }
            for(auto& it : threads){
                it.join();
            }
        });
        
        AFTER_ALL({
            forest::cut_tree("test");
        });
        
        IT("All expected records should be iterated", {
            for(auto& vec : tree_keys_check){
                unordered_set<string> keys;
                for(auto& it : vec){
                    keys.insert(it);
                }
                for(auto& it : should_contains){
                    EXPECT(keys.count(it)).toBe(1);
                }
            }
            EXPECT(num_assert).toBe(true);
        });
    });
});

SCENARIO_END


int main(){ return 0; }
