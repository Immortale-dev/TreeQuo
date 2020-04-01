#include "forest.hpp"


DESCRIBE("Test single thread", {
	
	srand(0);

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
		
		DESCRIBE_ONLY("Add `test` tree to the forest", {
			BEFORE_ALL({
				forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test", 10);
			});
			
			AFTER_ALL({
				forest::delete_tree("test");
			});
			
			DESCRIBE("Add 120 items to the tree", {
				BEFORE_ALL({
					for(int i=0;i<120;i++){
						forest::insert_leaf("test", "k"+std::to_string(i), forest::leaf_value("value_" + std::to_string(i*i)));
					}
				});
				
				IT("The value of `k25` should be equal to `value_625`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k25"))).toBe("value_625");
				});
				
				IT("The value of `k100` should be equal to `value_10000`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k100"))).toBe("value_10000");
				});
				
				IT("Tree should contain keys from `k0` to `k99`", {
					for(int i=0;i<100;i++){
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k" + std::to_string(i)))).toBe("value_" + std::to_string(i*i));
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
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "p" + std::to_string(it)))).toBe("val_" + std::to_string(it*it));
					}
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
		DESCRIBE_ONLY("Add `test` tree to the forest", {
			BEFORE_ALL({
				forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test", 10);
			});
			
			AFTER_ALL({
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
				
				IT("The value of `k25` should be equal to `value_625`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k25"))).toBe("value_625");
				});
				
				IT("The value of `k100` should be equal to `value_10000`", {
					EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k100"))).toBe("value_10000");
				});
				
				IT("Tree should contain keys from `k0` to `k99`", {
					for(int i=0;i<100;i++){
						EXPECT(forest::read_leaf_item(forest::find_leaf("test", "k" + std::to_string(i)))).toBe("value_" + std::to_string(i*i));
					}
				});
			});
			
			DESCRIBE("Add 100 random leafs in 100 threads to the tree and check the value of all items in 100 threads", {
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
							if( forest::read_leaf_item(forest::find_leaf("test", "p" + std::to_string(ind))) != "val_" + std::to_string(ind*ind) ){
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
		});
	});
});
