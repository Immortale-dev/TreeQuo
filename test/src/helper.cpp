#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>

int dir_count(std::string path)
{
    DIR *dp;
    int i = 0;
    struct dirent *ep;     
    dp = opendir (path.c_str());

    if (dp != NULL)
    {
        while ( (ep = readdir (dp)) )
            i++;

        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");
    
    return i-2;
}

string to_str(int a)
{
    string ret;
    while(a > 0){
        ret.push_back(a%26+'a');
        a/=26;
    }
    while(ret.size()<10){
        ret.push_back('a');   
    }
    reverse(ret.begin(),ret.end());
    return ret;
}

void config_high()
{
	forest::config_root_factor(100);
	forest::config_default_factor(100);
	forest::config_intr_cache_length(30);
	forest::config_leaf_cache_length(100);
	forest::config_tree_cache_length(10);
	forest::config_cache_bytes(256);
	forest::config_chunk_bytes(512);
	forest::config_opened_files_limit(100);
	forest::config_savior_queue_size(200);
	forest::config_save_schedule_mks(20000);
}

void config_low()
{
	forest::config_root_factor(3);
	forest::config_default_factor(3);
	forest::config_intr_cache_length(3);
	forest::config_leaf_cache_length(3);
	forest::config_tree_cache_length(3);
	forest::config_cache_bytes(128);
	forest::config_chunk_bytes(256);
	forest::config_opened_files_limit(10);
	forest::config_savior_queue_size(20);
	forest::config_save_schedule_mks(20000);
}

string read_leaf(forest::DetachedLeaf item)
{
	int sz = item->size();
	char* buf = new char[sz];
	auto reader = item->get_reader();
	reader.read(buf,sz);
	string ret(buf,sz);
	delete[] buf;
	return ret;
}
