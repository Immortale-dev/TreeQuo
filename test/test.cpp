#include <iostream>

#include "db.h"

using namespace std;


int main()
{
	
	DB db;
    
    db.bloom("tmp");
    
    db.create_qtree(TREE_TYPES::KEY_INT, "test_int");
	
	cout << "WOOHOOO" << endl;
	
	return 0;
}
