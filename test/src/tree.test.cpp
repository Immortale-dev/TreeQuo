#include "db.h"

DESCRIBE("Test single thread", {
	
	srand(0);

	DESCRIBE("Initialize forest at tmp/t1", {
		
		DB forest;
		
		BEFORE_ALL({
			forest.bloom("tmp/t1");
		});
		
		AFTER_ALL({
			forest.fold();
			// Remove dirs?
		});
		
		DESCRIBE("Add 100 trees", {
			BEFORE_ALL({
				for(int i=0;i<100;i++){
					forest.create_tree(TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(i));
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t1");
				EXPECT(fc).toBeGreaterThanOrEqual(202);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
		});
		
		DESCRIBE("Remove 100 trees", {
			BEFORE_ALL({
				for(int i=0;i<100;i++){
					forest.delete_tree("test_string_tree_"+to_string(i));
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
					forest.create_tree(TREE_TYPES::KEY_STRING, "tree_"+to_string(nums[i]));
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t1");
				EXPECT(fc).toBeGreaterThanOrEqual(cnt*2+2);
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove all trees in random shuffle", {
				BEFORE_ALL({
					for(int i=0;i<cnt;i++){
						swap(nums[i],nums[rand()%cnt]);
					}
					for(int i=0;i<cnt;i++){
						forest.delete_tree("tree_"+to_string(nums[i]));
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
	//
});
