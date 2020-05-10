#include "forest.hpp"

DESCRIBE("Test single thread", {
	
	srand(time(0));

	DESCRIBE("Initialize forest at tmp/t1", {
		
		//DB forest;
		
		BEFORE_ALL({
			forest::bloom("tmp/t1");
		});
		
		AFTER_ALL({
			forest::fold();
			// Remove dirs?
		});
		
		DESCRIBE("Add 100 trees", {
			BEFORE_ALL({
				for(int i=0;i<100;i++){
					forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(i));
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t1");
				EXPECT(fc).toBeGreaterThanOrEqual(102);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
		});
		
		DESCRIBE("Remove 100 trees", {
			BEFORE_ALL({
				for(int i=0;i<100;i++){
					forest::delete_tree("test_string_tree_"+to_string(i));
				}
			});
			
			IT("Trees should be deleted", {
				int fc = dir_count("tmp/t1");
				EXPECT(fc).toBeLessThanOrEqual(2);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
		});
		
		DESCRIBE("Add 100 trees in random shuffle", {
			vector<int> nums;
			int cnt = 100;
			
			BEFORE_ALL({
				for(int i=0;i<cnt;i++){
					nums.push_back(i);
				}
				for(int i=0;i<cnt;i++){
					swap(nums[i],nums[rand()%cnt]);
				}
				for(int i=0;i<cnt;i++){
					forest::create_tree(forest::TREE_TYPES::KEY_STRING, "tree_"+to_string(nums[i]));
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t1");
				EXPECT(fc).toBeGreaterThanOrEqual(cnt+2);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove all trees in random shuffle", {
				BEFORE_ALL({
					for(int i=0;i<cnt;i++){
						swap(nums[i],nums[rand()%cnt]);
					}
					for(int i=0;i<cnt;i++){
						forest::delete_tree("tree_"+to_string(nums[i]));
					}
				});
				
				IT("Trees should be deleted", {
					int fc = dir_count("tmp/t1");
					EXPECT(fc).toBeLessThanOrEqual(2);
					INFO_PRINT("Dirs count: " + to_string(fc));
				});
			});
		});
		
		DESCRIBE("Add `test` tree to the forest", {
			BEFORE_EACH({
				forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test", 10);
			});
			
			AFTER_EACH({
				forest::delete_tree("test");
			});
			
			DESCRIBE("Add 120 items to the tree", {
				BEFORE_ALL({
					for(int i=0;i<120;i++){
						forest::insert_leaf("test", "k"+std::to_string(i), forest::leaf_value("value_" + std::to_string(i*i)));
					}
				});
				
				IT("Tree should contain keys from `k0` to `k99`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k25")->val())).toBe("value_625");
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k100")->val())).toBe("value_10000");
					for(int i=0;i<100;i++){
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k" + std::to_string(i))->val())).toBe("value_" + std::to_string(i*i));
					}
				});
			});
			
			DESCRIBE("Add 100 random random leafs to the tree", {
				unordered_set<int> check;
				BEFORE_ALL({
					for(int i=0;i<100;i++){
						int rnd = rand()%10000 + 200;
						if(!check.count(rnd)){
							check.insert(rnd);
							forest::insert_leaf("test", "p"+std::to_string(rnd), forest::leaf_value("val_" + std::to_string(rnd*rnd)));
						}
					}
				});
				
				IT("all leafs should exist in tree", {
					for(auto &it : check){
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "p" + std::to_string(it))->val())).toBe("val_" + std::to_string(it*it));
					}
				});
			});
			
			DESCRIBE("Add 150 items in random shuffle", {
				
				unordered_set<int> check;
				int num_of_items;
				BEFORE_ALL({
					for(int i=0;i<150;i++){
						int rnd = rand()%10000 + 200;
						if(!check.count(rnd)){
							check.insert(rnd);
							forest::insert_leaf("test", "p"+std::to_string(rnd), forest::leaf_value("val_" + std::to_string(rnd*rnd)));
						}
					}
					num_of_items = check.size();
				});
				
				DESCRIBE("Move throug all of the items with ++it mode", {
					vector<string> tree_keys;
					int time_for_trave;
					
					BEFORE_ALL({
						auto it = forest::find_leaf("test");
						
						chrono::time_point t1 = chrono::system_clock::now();
						
						do{
							tree_keys.push_back(it->key());
						}while(it->move_forward());
						
						chrono::time_point t2 = chrono::system_clock::now();
						
						time_for_trave = chrono::duration_cast<chrono::milliseconds>(t2-t1).count();
					});
					
					IT("All keys should be iterated", {
						vector<string> set_keys;
						for(auto& it : check){
							set_keys.push_back("p"+std::to_string(it));
						}
						
						sort(set_keys.begin(), set_keys.end());
						sort(tree_keys.begin(), tree_keys.end());
						
						INFO_PRINT("Time for Travel: " + to_string(time_for_trave) + " ms");
						
						EXPECT(set_keys.size()).toBe(tree_keys.size());
						EXPECT(set_keys).toBeIterableEqual(tree_keys);
					});
				});
			});
		});
	});
});

DESCRIBE("Test multi threads", {
	DESCRIBE("Initialize forest at tmp/t2", {
		
		mutex mt;
		
		BEFORE_ALL({
			forest::bloom("tmp/t2");
		});
		
		AFTER_ALL({
			forest::fold();
			// Remove dirs?
		});
		
		DESCRIBE("Add 100 trees in 10 threads", {
			BEFORE_ALL({
				vector<thread> trds;
				for(int i=0;i<10;i++){
					thread t([&mt](int i){
						while(i<100){
							//mt.lock();
							forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(i));
							i+=10;
							//mt.unlock();
						}
					},i);
					trds.push_back(move(t));
				}
				for(int i=0;i<10;i++){
					trds[i].join();
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t2");
				EXPECT(fc).toBeGreaterThanOrEqual(102);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove 100 trees in 10 threads", {
				BEFORE_ALL({
					vector<thread> trds;
					for(int i=0;i<10;i++){
						thread t([&mt](int i){
							while(i<100){
								//mt.lock();
								forest::delete_tree("test_string_tree_"+to_string(i));
								i+=10;
								//mt.unlock();
							}
						},i);
						trds.push_back(move(t));
					}
					for(int i=0;i<10;i++){
						trds[i].join();
					}
				});
				
				IT("Trees should be deleted", {
					int fc = dir_count("tmp/t2");
					EXPECT(fc).toBeLessThanOrEqual(2);
					INFO_PRINT("Dirs count: " + to_string(fc));
				});
			});
		});
		
		
		
		DESCRIBE("Add 100 trees in random shuffle", {
			vector<int> nums;
			int cnt = 100;
			
			BEFORE_ALL({
				for(int i=0;i<cnt;i++){
					nums.push_back(i);
				}
				for(int i=0;i<cnt;i++){
					swap(nums[i],nums[rand()%cnt]);
				}
				
				vector<thread> trds;
				
				for(int i=0;i<100;i++){
					thread t([&cnt,&nums,&mt](int i){
						while(i<cnt){
							//mt.lock();
							forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(nums[i]));
							i+=100;
							//mt.unlock();
						}
					},i);
					trds.push_back(move(t));
				}
				for(int i=0;i<100;i++){
					trds[i].join();
				}
			});
			
			IT("Trees should be created", {
				DBFS::File* f = new DBFS::File("_root");
				int a;
				f->read(a);
				EXPECT(a).toBe(100);
				INFO_PRINT("Trees created: " + to_string(a));
				f->close();
				delete f;
				int fc = dir_count("tmp/t2");
				EXPECT(fc).toBeGreaterThanOrEqual(cnt+2);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove all trees in random shuffle in 10 threads", {
				BEFORE_ALL({
					for(int i=0;i<cnt;i++){
						swap(nums[i],nums[rand()%cnt]);
					}
					vector<thread> trds;
					for(int i=0;i<100;i++){
						thread t([&cnt,&nums,&mt](int i){
							while(i<cnt){
								//mt.lock();
								forest::delete_tree("test_string_tree_"+to_string(nums[i]));
								i+=100;
								//mt.unlock();
							}
						},i);
						trds.push_back(move(t));
					}
					for(int i=0;i<100;i++){
						trds[i].join();
					}
				});
				
				IT("Trees should be deleted", {
					int fc = dir_count("tmp/t2");
					EXPECT(fc).toBeLessThanOrEqual(2);
					INFO_PRINT("Dirs count: " + to_string(fc));
				});
			});
		});
		
		// Leaf insertions
		DESCRIBE("Add `test` tree to the forest", {
			BEFORE_EACH({
				forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test", 10);
			});
			
			AFTER_EACH({
				forest::delete_tree("test");
			});
			
			DESCRIBE("Add 120 items in 10 threads to the tree", {
				BEFORE_ALL({
					vector<thread> trds;
					for(int i=0;i<10;i++){
						thread t([](int ind){
							while(ind<120){
								forest::insert_leaf("test", "k"+std::to_string(ind), forest::leaf_value("value_" + std::to_string(ind*ind)));
								ind += 10;
							}
						}, i);
						trds.push_back(move(t));
					}
					for(int i=0;i<10;i++){
						trds[i].join();
					}
				});
				
				IT("Tree should contain keys from `k0` to `k99`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k25")->val())).toBe("value_625");
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k100")->val())).toBe("value_10000");
					for(int i=0;i<100;i++){
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k" + std::to_string(i))->val())).toBe("value_" + std::to_string(i*i));
					}
				});
			});
			
			DESCRIBE("Add 100 random items in 100 threads to the tree and check the value of all items in 100 threads", {
				unordered_set<int> check;
				bool good = true;
				BEFORE_ALL({
					vector<thread> trds;
					for(int i=0;i<100;i++){
						int rnd = rand()%10000 + 200;
						if(!check.count(rnd)){
							check.insert(rnd);
							thread t([](int ind){								
								forest::insert_leaf("test", "p"+std::to_string(ind), forest::leaf_value("val_" + std::to_string(ind*ind)));
							}, rnd);
							trds.push_back(move(t));
						}
					}
					for(auto &it : trds){
						it.join();
					}
					trds.resize(0);
					for(auto &it : check){
						thread t([&good](int ind){		
							auto it = forest::find_leaf("test", "p" + std::to_string(ind));						
							if( forest::read_leaf_item(it->val()) != "val_" + std::to_string(ind*ind) ){
								good = false;
							}
						}, it);
						trds.push_back(move(t));
					}
					for(auto &it : trds){
						it.join();
					}
				});
				
				IT("all leafs should exist in tree", {
					EXPECT(good).toBe(true);
				});
			});
			
			DESCRIBE("Add 150 items in random shuffle", {
				unordered_set<int> check;
				int num_of_items;
				BEFORE_EACH({
					check.clear();
					for(int i=0;i<150;i++){
						int rnd = rand()%10000 + 200;
						if(!check.count(rnd)){
							check.insert(rnd);
							forest::insert_leaf("test", "p"+std::to_string(rnd), forest::leaf_value("val_" + std::to_string(rnd*rnd)));
						}
					}
					num_of_items = check.size();
					//assert(false);
				});
				
				
				DESCRIBE("Move throug all of the items in 2 threads one to another", {
					vector<string> tree_keys_f, tree_keys_b;
					
					BEFORE_ALL({
						auto itf = forest::find_leaf("test", forest::RECORD_POSITION::BEGIN);
						auto itb = forest::find_leaf("test", forest::RECORD_POSITION::END);

						thread t1([&itf, &tree_keys_f](){	
							do{
								tree_keys_f.push_back(itf->key());
							}while(itf->move_forward());
						});
						
						thread t2([&itb, &tree_keys_b](){
							do{
								tree_keys_b.push_back(itb->key());
							}while(itb->move_back());
						});
						
						t1.join();
						t2.join();
					});
					
					IT("All keys should be iterated", {
						vector<string> set_keys;
						for(auto& it : check){
							set_keys.push_back("p"+std::to_string(it));
						}
						
						sort(set_keys.begin(), set_keys.end());
						sort(tree_keys_f.begin(), tree_keys_f.end());
						sort(tree_keys_b.begin(), tree_keys_b.end());
						
						EXPECT(set_keys.size()).toBe(tree_keys_f.size());
						EXPECT(set_keys.size()).toBe(tree_keys_b.size());
						
						EXPECT(set_keys).toBeIterableEqual(tree_keys_f);
						EXPECT(set_keys).toBeIterableEqual(tree_keys_b);
					});
				});
				
				DESCRIBE("Move throug all of the items in 100 threads in random shuffle", {
					
					int tests_count = 100;
					
					vector<vector<string> > tree_keys_check(tests_count);
					vector<thread> threads;
					
					BEFORE_ALL({
						for(int i=0;i<tests_count;i++){
							thread t([&tree_keys_check](int index){
								auto it = forest::find_leaf("test", (index%2 ? forest::RECORD_POSITION::BEGIN : forest::RECORD_POSITION::END));
								do{
									tree_keys_check[index].push_back(it->key());
									if( (index%2 == 1 && !it->move_forward()) || (index%2 == 0 && !it->move_back()) ){
										break;
									}
								}while(true);
							}, i);
							threads.push_back(move(t));
						}
						for(auto& it : threads){
							it.join();
						}
					});
					
					IT("All iterators should successfully move throug the all records", {
						vector<string> set_keys;
						for(auto& it : check){
							set_keys.push_back("p"+std::to_string(it));
						}
						sort(set_keys.begin(), set_keys.end());
						
						for(auto& it : tree_keys_check){
							sort(it.begin(), it.end());
							EXPECT(set_keys).toBeIterableEqual(it);
						}
					});
				});
				
				DESCRIBE("Do 200 operations in 200 threads", {
					int tests_count = 200;
					
					vector<vector<string> > tree_keys_check;
					vector<thread> threads;
					
					unordered_set<string> should_contains;
					queue<int> q;
					
					vector<string> complete;
					
					bool num_assert = true;
					
					BEFORE_ALL({
						for(auto& it : check){
							q.push(it);
							should_contains.insert("p"+std::to_string(it));
						}
						mutex m;
						
						for(int i=0;i<tests_count;i++){
							int rnd = rand()%10000 + 11200;
							thread t([&m, &q, &check, &tree_keys_check, &should_contains, &complete, &num_assert](int index, int rnd){
								int cs = index%8;
								if(cs == 1){
									auto it = forest::find_leaf("test", forest::RECORD_POSITION::BEGIN);
									vector<string> keys;
									do{
										string key = it->key();
										keys.push_back(key);
										auto val = it->val();
										string sval = forest::read_leaf_item(val);
										
										if(sval.substr(0,3) == "new"){
											continue;
										}
										int k, v;
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
								}
								else if(cs == 2){
									auto it = forest::find_leaf("test", forest::RECORD_POSITION::END);
									vector<string> keys;
									do{
										string key = it->key();
										keys.push_back(key);
										auto val = it->val();
										string sval = forest::read_leaf_item(val);
										
										if(sval.substr(0,3) == "new"){
											continue;
										}
										int k, v;
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
								}
								else if(cs == 3){
									// doesn't matter
								}
								else if(cs == 4){
									int p;
									{
										lock_guard<mutex> lock(m);
										p = q.front();
										q.pop();
									}
									auto it = forest::find_leaf("test", "p"+std::to_string(p));
									do{
										it->val();
									}while(it->move_forward());
								}
								else if(cs == 5){
									int p;
									{
										lock_guard<mutex> lock(m);
										p = q.front();
										q.pop();
									}
									auto it = forest::find_leaf("test", "p"+std::to_string(p));
									do{
										it->val();
									}while(it->move_back());
								}
								else if(cs == 6){
									int p;
									{
										lock_guard<mutex> lock(m);
										p = q.front();
										q.pop();
									}
									forest::update_leaf("test", "p"+std::to_string(p), forest::leaf_value("new_" + std::to_string(rnd*rnd)));
								}
								else if(cs == 7){
									int p;
									{
										lock_guard<mutex> lock(m);
										p = q.front();
										q.pop();
									}
									forest::erase_leaf("test", "p"+std::to_string(p));
									lock_guard<mutex> lock(m);
									should_contains.erase("p"+std::to_string(p));
									complete.push_back("erase");
								}
								else{
									{
										lock_guard<mutex> lock(m);
										if(check.count(rnd)){
											return;
										}
										check.insert(rnd);
									}
									forest::insert_leaf("test", "p"+std::to_string(rnd), forest::leaf_value("val_" + std::to_string(rnd*rnd)));
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
				
				DESCRIBE("Comparing time for insert on free and busy tree", {
					
					int time_busy, time_free;
					
					DESCRIBE("For free tree", {
						IT("should quickly insert all values", {
							chrono::time_point p1 = chrono::system_clock::now();				
							for(int i=0;i<10;i++){
								int rnd = rand()%10000 + 11200;
								forest::insert_leaf("test", "c"+std::to_string(rnd), forest::leaf_value("val111"));
							}
							chrono::time_point p2 = chrono::system_clock::now();
							time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
							TEST_SUCCEED();
							INFO_PRINT("Time For Insert: " + to_string(time_free));
						});
					});
					
					DESCRIBE("For busy tree", {
						vector<thread> threads;
						bool finish = false;
						
						BEFORE_ALL({						
							for(int i=0;i<10;i++){
								thread t([&finish](int index){
									int dir = index%2;
									auto it = forest::find_leaf("test", dir ? forest::RECORD_POSITION::BEGIN : forest::RECORD_POSITION::END);
									while(!finish){
										while( (dir && it->move_forward()) || (!dir && it->move_back()) ){
											// ok
										}
										this_thread::sleep_for(chrono::milliseconds(1));
									}
								}, i);
								threads.push_back(move(t));
							}
						});
						
						IT("should quickly insert all values", {
							chrono::time_point p1 = chrono::system_clock::now();				
							for(int i=0;i<10;i++){
								int rnd = rand()%10000 + 11200;
								forest::insert_leaf("test", "c"+std::to_string(rnd), forest::leaf_value("val111"));
							}
							chrono::time_point p2 = chrono::system_clock::now();
							time_busy = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
							finish = true;
							for(auto& it : threads){
								it.join();
							}
							TEST_SUCCEED();
							INFO_PRINT("Time For Insert: " + to_string(time_busy));
						});
					});
					
					DESCRIBE("Analyzing the difference", {
						IT("The difference should not be significant", {
							EXPECT(time_free*2).toBeGreaterThan(time_busy);
						});
					});
				});
			});
		});
	});
});
