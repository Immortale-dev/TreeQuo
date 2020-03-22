#include "forest.hpp"


DESCRIBE_SKIP("Test single thread", {
	
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
		
		DESCRIBE_SKIP("Add 100 trees in 10 threads", {
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
	});
});
