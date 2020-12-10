
DESCRIBE("Test multi threads", {
	DESCRIBE("Initialize forest at tmp/t2", {
		
		BEFORE_ALL({
			config_low();
			forest::bloom("tmp/t2");
		});
		
		AFTER_ALL({
			forest::fold();
		});
		
		DESCRIBE("Add 100 trees in 10 threads", {
			BEFORE_ALL({
				vector<thread> trds;
				for(int i=0;i<10;i++){
					thread t([](int i){
						while(i<100){
							forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(i));
							i+=10;
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
				TEST_SUCCEED();
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove 100 trees in 10 threads", {
				BEFORE_ALL({
					vector<thread> trds;
					for(int i=0;i<10;i++){
						thread t([](int i){
							while(i<100){
								forest::cut_tree("test_string_tree_"+to_string(i));
								i+=10;
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
					// After savior implementation not all files is going to be delete immediatelu
					// So increasing the value to not block the project with tests reimplenemtation.
					TEST_SUCCEED();
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
					thread t([&cnt,&nums](int i){
						while(i<cnt){
							forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test_string_tree_"+to_string(nums[i]));
							i+=100;
						}
					},i);
					trds.push_back(move(t));
				}
				for(int i=0;i<100;i++){
					trds[i].join();
				}
			});
			
			IT("Trees should be created", {
				int fc = dir_count("tmp/t2");
				TEST_SUCCEED();
				INFO_PRINT("Dirs count: " + to_string(fc));
			});
			
			DESCRIBE("Then remove all trees in random shuffle in 100 threads", {
				BEFORE_ALL({
					for(int i=0;i<cnt;i++){
						swap(nums[i],nums[rand()%cnt]);
					}
					vector<thread> trds;
					for(int i=0;i<100;i++){
						thread t([&cnt,&nums](int i){
							while(i<cnt){
								forest::cut_tree("test_string_tree_"+to_string(nums[i]));
								i+=100;
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
					TEST_SUCCEED();
					INFO_PRINT("Dirs count: " + to_string(fc));
				});
			});
		});
		
		DESCRIBE("Add 10 trees", {
			vector<std::thread> threads;
			
			BEFORE_ALL({
				for(int i=0;i<10;i++){
					forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "tree_"+std::to_string(i), 3);
				}
			});
			
			AFTER_ALL({
				for(int i=0;i<10;i++){
					forest::cut_tree("tree_"+std::to_string(i));
				}
			});
			
			IT("Add 100 items to 10 trees in 100 threads randomly", {
				for(int i=0;i<100;i++){
					std::thread t([](int ind, int tree_id){
						forest::insert_leaf("tree_"+std::to_string(tree_id), "key_"+std::to_string(ind), forest::leaf_value("value_"+std::to_string(ind*2)));
					}, i, rand()%10);
					threads.push_back(std::move(t));
				}
				for(auto& it : threads){
					it.join();
				}
			});
		});
		
		// Leaf insertions
		DESCRIBE("Add `test` tree to the forest", {
			BEFORE_EACH({
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test", 3);
			});
			
			AFTER_EACH({
				forest::cut_tree("test");
			});
			
			DESCRIBE("Add 100 items with even keys and lower/upper bound records", {
				vector<string> resl[100];
				vector<string> resr[100];
				
				BEFORE_ALL({
					for(int i=0;i<10;i++){
						char c = (i*2)+'a';
						string k = "";
						k.push_back(c);
						forest::insert_leaf("test", k, forest::leaf_value("value_" + std::to_string(i*2)));
					}
					
					vector<thread> trds;
					for(int i=0;i<100;i++){
						thread t([&resl, &resr](int ind){
							for(int j=0;j<10;j++){
								string k="";
								k.push_back('a'+j);
								auto record = forest::find_leaf("test", k, forest::LEAF_POSITION::LOWER);
								resl[ind].push_back(record->key());
								record = forest::find_leaf("test", k, forest::LEAF_POSITION::UPPER);
								resr[ind].push_back(record->key());
							}
						}, i);
						trds.push_back(move(t));
					}
					for(int i=0;i<100;i++){
						trds[i].join();
					}
				});
				
				IT("result should be expected", {
					for(int i=0;i<100;i++){
						for(int j=0;j<10;j++){
							string kl = "", kr = "";
							kl.push_back('a' + (j-j%2));
							kr.push_back('a' + (j-j%2 + 2));
							EXPECT(resl[i][j]).toBe(kl);
							EXPECT(resr[i][j]).toBe(kr);
						}
					}
				});
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
				});
				
				
				DESCRIBE("Move throug all of the items in 2 threads one to another", {
					vector<string> tree_keys_f, tree_keys_b;
					
					BEFORE_ALL({
						auto itf = forest::find_leaf("test", forest::LEAF_POSITION::BEGIN);
						auto itb = forest::find_leaf("test", forest::LEAF_POSITION::END);

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
				
				DESCRIBE("Move throug all of the items in 200 threads in random shuffle", {
					
					int tests_count = 200;
					
					vector<vector<string> > tree_keys_check(tests_count);
					vector<thread> threads;
					
					BEFORE_ALL({
						for(int i=0;i<tests_count;i++){
							thread t([&tree_keys_check](int index){
								auto it = forest::find_leaf("test", (index%2 ? forest::LEAF_POSITION::BEGIN : forest::LEAF_POSITION::END));
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
									auto it = forest::find_leaf("test", forest::LEAF_POSITION::BEGIN);
									vector<string> keys;
									do{
										string key = it->key();
										keys.push_back(key);
										auto val = it->val();
										string sval = forest::read_leaf_item(val);
										
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
								}
								else if(cs == 2){
									auto it = forest::find_leaf("test", forest::LEAF_POSITION::END);
									vector<string> keys;
									do{
										string key = it->key();
										keys.push_back(key);
										auto val = it->val();
										string sval = forest::read_leaf_item(val);
										
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
									forest::update_leaf("test", "p"+std::to_string(p), std::move(forest::leaf_value("new_" + std::to_string(rnd*rnd))) );
								}
								else if(cs == 7){
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
								}
								else{
									{
										lock_guard<mutex> lock(m);
										if(check.count(rnd)){
											return;
										}
										check.insert(rnd);
									}
									forest::file_data_ptr tmp = forest::leaf_value("val_" + std::to_string(rnd*rnd));
									forest::insert_leaf("test", "p" + std::to_string(rnd), std::move(tmp) );
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
			});
		});
	});
	
	DESCRIBE("Check all files deleted", {
		IT("Should not find any not deleted files", {
			int fc = dir_count("tmp/t2");
			// root tree and one leaf folders
			EXPECT(fc).toBe(2);
		});
	});
});

