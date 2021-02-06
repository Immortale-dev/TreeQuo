

DESCRIBE("Initialize forest at tmp/perf", {
	DESCRIBE("Performance Tests", {
				
		int time_free;
		chrono::high_resolution_clock::time_point p1,p2;
		
		DESCRIBE("Single Thread Small Data Pool", {
			BEFORE_ALL({
				config_high();
				forest::bloom("tmp/t2");
			
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test_else", 500);
			});
			
			int rec_count = 100000;
			
			IT("Insert 100000 items [0,100000)", {
				p1 = chrono::system_clock::now();
				forest::Tree tree = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					forest::insert_leaf(tree, to_str(i), forest::make_leaf("some pretty basic value to insert into the database"));
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Insert: " + to_string(time_free) + "ms");
			});
			
			IT("Get all items independently", {
				p1 = chrono::system_clock::now();
				forest::Tree t = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					auto rc = forest::find_leaf(t, to_str(i));
					EXPECT(read_leaf(rc->val())).toBe("some pretty basic value to insert into the database");
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Find: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values", {
				p1 = chrono::system_clock::now();
				auto rc = forest::find_leaf("test_else", forest::LEAF_POSITION::BEGIN);
				int cnt = 0;
				do{
					EXPECT(read_leaf(rc->val())).toBe("some pretty basic value to insert into the database");
					cnt++;
				}while(rc->move_forward());
				EXPECT(cnt).toBe(rec_count);
				INFO_PRINT("CNT: " + std::to_string(cnt));
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Move: " + to_string(time_free) + "ms");
			});
			
			IT("Remove all records", {
				p1 = chrono::system_clock::now();
				forest::Tree t = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					forest::remove_leaf(t, to_str(i));
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Remove: " + to_string(time_free) + "ms");
			});
			
			AFTER_ALL({
				forest::cut_tree("test_else");
				
				forest::fold();
			});
		});
		
		DESCRIBE("Single Thread Big Data Pool", {
			BEFORE_ALL({
				config_high();
				forest::bloom("tmp/t2");
			
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test_else", 1000);
			});
			
			int rec_count = 1000000;
			
			IT_ONLY("Insert 1000000 items [0,1000000)", {
				p1 = chrono::system_clock::now();
				forest::Tree tree = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					forest::insert_leaf(tree, to_str(i), forest::make_leaf("value"));
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Insert: " + to_string(time_free) + "ms");
				
				#ifdef DEBUG_PERF
				INFO_PRINT("d_enter_time: " + to_string(h_enter/1000000));
				INFO_PRINT("d_leave_time: " + to_string(h_leave/1000000));
				INFO_PRINT("d_insert_time: " + to_string(h_insert/1000000));
				INFO_PRINT("d_remove_time: " + to_string(h_remove/1000000));
				INFO_PRINT("d_reserve_time: " + to_string(h_reserve/1000000));
				INFO_PRINT("d_release_time: " + to_string(h_release/1000000));
				INFO_PRINT("d_leaf_insert: " + to_string(h_l_insert/1000000));
				INFO_PRINT("h_l_ref: " + to_string(h_l_ref/1000000));
				INFO_PRINT("h_save_base: " + to_string(h_save_base/1000000));
				INFO_PRINT("h_saviour_blocking: " + to_string(h_blocking/1000000));
				#endif
			});
			
			IT("Get all items independently", {
				p1 = chrono::system_clock::now();
				forest::Tree t = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					auto rc = forest::find_leaf(t, to_str(i));
					EXPECT(read_leaf(rc->val())).toBe("value");
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Find: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values", {
				p1 = chrono::system_clock::now();
				auto rc = forest::find_leaf("test_else", forest::LEAF_POSITION::BEGIN);
				int cnt = 0;
				do{
					EXPECT(read_leaf(rc->val())).toBe("value");
					cnt++;
				}while(rc->move_forward());
				EXPECT(cnt).toBe(rec_count);
				INFO_PRINT("CNT: " + std::to_string(cnt));
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Move: " + to_string(time_free) + "ms");
			});
			
			IT("Remove all records", {
				p1 = chrono::system_clock::now();
				forest::Tree t = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					forest::remove_leaf(t, to_str(i));
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Remove: " + to_string(time_free) + "ms");
			});
			
			AFTER_ALL({
				forest::cut_tree("test_else");
				
				forest::fold();
			});
		});
		
		
		DESCRIBE("Multithread work with single tree", {
			BEFORE_ALL({
				config_high();
				forest::bloom("tmp/t2");
			
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "test_else", 500);
			});
			
			int rec_count = 1000000;
			
			IT("Insert 1000000 items [0,1000000) in 8 threads", {
				p1 = chrono::system_clock::now();
				forest::Tree tree = forest::find_tree("test_else");
				
				vector<std::thread> thrds;
				for(int t=0;t<8;t++){
					thrds.push_back(thread([rec_count, tree](int t){
						int start = rec_count/8*t;
						int end = start + rec_count/8;
						for(int i=start;i<end;i++){
							forest::insert_leaf(tree, to_str(i), forest::make_leaf("some pretty basic value to insert into the database"));
						}
					}, t));
				}
				
				for(auto& t : thrds){
					t.join();
				}
				
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Insert: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values in 8 threads (w/o overlapping data)", {
				p1 = chrono::system_clock::now();
				
				vector<std::thread> thrds;
				for(int t=0;t<8;t++){
					thrds.push_back(thread([rec_count](int t){
						int start = rec_count/8*t;
						int count = rec_count/8;
						
						auto rc = forest::find_leaf("test_else", to_str(start));
						do{
							EXPECT(read_leaf(rc->val())).toBe("some pretty basic value to insert into the database");
							rc->move_forward();
						}while(--count);
					}, t));
				}
				
				for(auto& t : thrds){
					t.join();
				}
				
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Move: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values in 8 threads (with overlapping data)", {
				p1 = chrono::system_clock::now();
				
				vector<std::thread> thrds;
				for(int t=0;t<8;t++){
					thrds.push_back(thread([rec_count](int t){
						int start = rec_count/8*t;
						
						auto rc = forest::find_leaf("test_else", to_str(start));
						do{
							EXPECT(read_leaf(rc->val())).toBe("some pretty basic value to insert into the database");
						}while(rc->move_forward());
					}, t));
				}
				
				for(auto& t : thrds){
					t.join();
				}
				
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Move: " + to_string(time_free) + "ms");
			});
			
			AFTER_ALL({
				forest::cut_tree("test_else");
				
				forest::fold();
			});
		});
		
		
		DESCRIBE("Multithread work with multiple trees", {
			BEFORE_ALL({
				config_high();
				forest::bloom("tmp/t2");
				
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "tr0", 500);
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "tr1", 500);
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "tr2", 500);
				forest::plant_tree(forest::TREE_TYPES::KEY_STRING, "tr3", 500);
			});
			
			int rec_count = 1000000;
			
			IT("Insert 1000000 items [0,1000000)", {
				p1 = chrono::system_clock::now();
				
				vector<thread> thrds;
				for(int i=0;i<4;i++){
					thrds.push_back(thread([rec_count](int ind){
						forest::Tree tree = forest::find_tree("tr"+std::to_string(ind));
						for(int i=0;i<rec_count/4;i++){
							forest::insert_leaf(tree, to_str(i), forest::make_leaf("some pretty basic value to insert into the database"));
						}
					}, i));
				}
				
				for(auto& it : thrds){
					it.join();
				}
				
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Insert: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values in 4 threads (each thread move through its own tree)", {
				p1 = chrono::system_clock::now();
				
				vector<thread> thrds;
				for(int i=0;i<4;i++){
					thrds.push_back(thread([rec_count](int ind){
						auto rc = forest::find_leaf("tr"+std::to_string(ind), forest::LEAF_POSITION::BEGIN);
						int cnt = 0;
						do{
							EXPECT(read_leaf(rc->val())).toBe("some pretty basic value to insert into the database");
							cnt++;
						}while(rc->move_forward());
						EXPECT(cnt).toBe(rec_count/4);
					}, i));
				}
				
				for(auto& it : thrds){
					it.join();
				}
				
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Move: " + to_string(time_free) + "ms");
			});
			
			AFTER_ALL({
				forest::cut_tree("tr0");
				forest::cut_tree("tr1");
				forest::cut_tree("tr2");
				forest::cut_tree("tr3");
				
				forest::fold();
			});
		});
	});
});
