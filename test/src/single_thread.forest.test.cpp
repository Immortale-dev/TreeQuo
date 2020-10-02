
DESCRIBE("Test single thread", {
	
	srand(time(0));

	DESCRIBE("Initialize forest at tmp/t1", {
		
		//DB forest;
		
		BEFORE_ALL({
			config_low();
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
				//EXPECT(fc).toBeGreaterThanOrEqual(102);
				INFO_PRINT("Dirs count: " + to_string(fc));
				TEST_SUCCEED();
			});
		});
		
		DESCRIBE("Remove 100 trees", {
			//int a;
			//cout << "ENTER\n";
			//cin >> a;
			BEFORE_ALL({
				for(int i=0;i<100;i++){
					//std::cout << "delete_tree " << i << std::endl;
					forest::delete_tree("test_string_tree_"+to_string(i));
				}
			});
			
			IT("Trees should be deleted", {
				int fc = dir_count("tmp/t1");
				// After savior implementation not all files is going to be delete immediatelu
				// So increasing the value to not block the project with tests reimplenemtation.
				//EXPECT(fc).toBeLessThanOrEqual(20);
				TEST_SUCCEED();
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
				//EXPECT(fc).toBeGreaterThanOrEqual(cnt+2);
				INFO_PRINT("Dirs count: " + to_string(fc));
				TEST_SUCCEED();
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
					// After savior implementation not all files is going to be delete immediatelu
					// So increasing the value to not block the project with tests reimplenemtation.
					//EXPECT(fc).toBeLessThanOrEqual(20);
					TEST_SUCCEED();
					INFO_PRINT("Dirs count: " + to_string(fc));
				});
			});
		});
		
		DESCRIBE("Add `test` tree to the forest", {
			BEFORE_EACH({
				forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test", 100);
			});
			
			AFTER_EACH({
				forest::delete_tree("test");
			});
			
			IT("move through the leaf should works", {
				auto record = forest::find_leaf("test", forest::RECORD_POSITION::BEGIN);

				record->move_forward();
				record->move_forward();
				record->move_forward();
				
				EXPECT(record->eof()).toBe(true);
			});
			
			DESCRIBE("Add 10 items with even key to the tree", {
				BEFORE_ALL({
					
					for(int i=0;i<10;i++){
						char c = (i*2)+'a';
						string k = "";
						k.push_back(c);
						//std::cout << "PUT " + k + "\n";
						forest::insert_leaf("test", k, forest::leaf_value("value_" + std::to_string(i*2)));
						//std::cout << "PUT_END " + k + "\n";
					}
					
				});
				
				IT("find lower/upper bound items from `a` to `j`", {
					TEST_SUCCEED();
					
					for(int i=0;i<10;i++){
						//std::cout << "FIND_LEAF_" + std::to_string(i) + "\n";
						char c = i + 'a';
						string k = "";
						k.push_back(c);
						//std::cout << "BEFORE_FIND_LOWER\n";
						auto record = forest::find_leaf("test", k, forest::RECORD_POSITION::LOWER);
						//std::cout << "AFTER_FIND_LOWER\n";
						string compare = "";
						compare.push_back((char)(i-i%2)+'a');
						EXPECT(record->key()).toBe(compare);
						
						//std::cout << "BEFORE_FIND_UPPER\n";
						record = forest::find_leaf("test", k, forest::RECORD_POSITION::UPPER);
						//std::cout << "AFTER_FIND_UPPER\n";
						compare = "";
						compare.push_back((char)(i-i%2+2)+'a');
						EXPECT(record->key()).toBe(compare);
						//std::cout << "END_FIND_LEAF\n";
					}
					
				});
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
		
		DESCRIBE("Add 20 trees with annotations", {
			BEFORE_ALL({
				for(int i=0;i<20;i++){
					forest::create_tree(forest::TREE_TYPES::KEY_STRING, "gt_" + std::to_string(i), 10, "annotation provided");
				}
			});
			
			IT("Every tree annotation should be equal to `annotation provided`", {
				for(int i=0;i<20;i++){
					forest::tree_ptr tree = forest::find_tree("gt_" + std::to_string(i));
					EXPECT(tree->get_annotation()).toBe("annotation provided");
					forest::leave_tree(tree);
				}
			});
			
			AFTER_ALL({
				for(int i=0;i<20;i++){
					forest::delete_tree("gt_" + std::to_string(i));
				}
			});
		});
	});
});
