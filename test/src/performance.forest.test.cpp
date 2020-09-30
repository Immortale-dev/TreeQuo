DESCRIBE("Initialize forest at tmp/perf", {
	BEFORE_ALL({
		config_high();
		forest::bloom("tmp/t2");
	});
	
	AFTER_ALL({
		forest::fold();
	});
	
	DESCRIBE("Performance Tests", {
				
		int time_free;
		chrono::high_resolution_clock::time_point p1,p2;
		
		BEFORE_ALL({
			forest::create_tree(forest::TREE_TYPES::KEY_STRING, "test_else", 500);
		});
		
		AFTER_ALL({
			forest::delete_tree("test_else");
		});
		
		DESCRIBE("Ordered requests", {
			
			int rec_count = 100000;
			
			IT("Insert 100000 items [0,100000)", {
				p1 = chrono::system_clock::now();
				forest::tree_ptr tree = forest::find_tree("test_else");
				for(int i=0;i<rec_count;i++){
					int rnd = i;//rand()%1000000 + 11200;
					
					tree->insert(to_str(rnd), forest::leaf_value("some pretty basic value to insert into the database"));
				}
				forest::leave_tree(tree);
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Insert: " + to_string(time_free) + "ms");
			});
			
			IT("Get all items independently", {
				p1 = chrono::system_clock::now();
				for(int i=0;i<rec_count;i++){
					int rnd = i;
					auto rc = forest::find_leaf("test_else", to_str(rnd));
					EXPECT(forest::read_leaf_item(rc->val())).toBe("some pretty basic value to insert into the database");
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Find: " + to_string(time_free) + "ms");
			});
			
			IT("Move through the all values", {
				p1 = chrono::system_clock::now();
				auto rc = forest::find_leaf("test_else", forest::RECORD_POSITION::BEGIN);
				int cnt = 0;
				do{
					EXPECT(forest::read_leaf_item(rc->val())).toBe("some pretty basic value to insert into the database");
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
				for(int i=0;i<rec_count;i++){
					int rnd = i;
					forest::erase_leaf("test_else", to_str(rnd));
				}
				p2 = chrono::system_clock::now();
				time_free = chrono::duration_cast<chrono::milliseconds>(p2-p1).count();
				TEST_SUCCEED();
				INFO_PRINT("Time For Remove: " + to_string(time_free) + "ms");
			});
		});
	});
});
