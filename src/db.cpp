#include "db.h"

DB::DB()
{
	
}

DB::~DB()
{
	fold();
}

DB::DB(string path)
{
	bloom(path);
}

void DB::bloom(string path)
{
	if(blossomed)
		return;
		
	DBFS::root = path;
	if(!DBFS::exists(ROOT_TREE)){
		create_root_file();
	}
	
	blossomed = true;
}

void DB::fold()
{
	delete FOREST;
	blossomed = false;
}


/**
 * Root file contains: `${number_of_elements} ${root_branch_file_name} ${is_root_branch_leaf_or_not} \n `
 * Leaf file contains: `${count_of_items} \n ${item_key} ${item_length} ${item_body} \n ... \n `
 * Intr file contains: `${is_children_branches_are_leafs_or_not} ${count_of_child_nodes} \n ${key1} ${key2} {key3} ... \n ${path0} ${path1} ${path2} ${path3} ... \n `
 * 
 **/ 
void DB::create_root_file()
{
	DBFS::File* rf = DBFS::create();
	if(rf.fail()){
		throw DBException(DBException::ERRORS::CANNOT_CREATE_ROOT);
	}
	rf.write("0\n");
	string root_branch_name = rf.name();
	rf.close();
	delete rf;
	
	DBFS::File* f = DBFS::create(ROOT_TREE);
	if(f.fail()){
		throw DBException(DBException::ERRORS::CANNOT_CREATE_ROOT);
	}
	f->write("0 " + root_branch_name + " 1\n");
	f.slose();
	delete f;
}
