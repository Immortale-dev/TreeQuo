#include <iostream>

#define DEBUG

#include "db.h"

using namespace std;


int main()
{
	
	DB db;
    
    db.bloom("tmp");
    
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int0");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int1");
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int2");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int3");
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int4");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int5");
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int0-");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int1-");
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int2-");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int3-");
    //db.create_qtree(TREE_TYPES::KEY_INT, "test_int4-");
    //db.create_qtree(TREE_TYPES::KEY_STRING, "test_int5-");
    db.create_qtree(TREE_TYPES::KEY_INT, "testt_int0");
    db.create_qtree(TREE_TYPES::KEY_STRING, "tetst_int1");
    db.create_qtree(TREE_TYPES::KEY_INT, "tesat_int2");
    db.create_qtree(TREE_TYPES::KEY_STRING, "tesst_int3");
    db.create_qtree(TREE_TYPES::KEY_INT, "tedst_int4");
    db.create_qtree(TREE_TYPES::KEY_STRING, "tesxt_int5");
    db.create_qtree(TREE_TYPES::KEY_INT, "test_izznt0-");
    db.create_qtree(TREE_TYPES::KEY_STRING, "txxxest_int1-");
    db.create_qtree(TREE_TYPES::KEY_INT, "test_icccnt2-");
    db.create_qtree(TREE_TYPES::KEY_STRING, "testv_int3-");
    db.create_qtree(TREE_TYPES::KEY_INT, "test_ibnt4-");
    db.create_qtree(TREE_TYPES::KEY_STRING, "tenst_int5-");
    db.create_qtree(TREE_TYPES::KEY_INT, "test_int6");
    db.create_qtree(TREE_TYPES::KEY_STRING, "test_int7");
	
	cout << "WOOHOOO" << endl;
	
	return 0;
}
