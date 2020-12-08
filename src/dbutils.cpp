#include "dbutils.hpp"

namespace forest{
	std::atomic<int> opened_files_count = 0;
	std::mutex opened_files_m;
	std::condition_variable opened_files_cv;
}

forest::node_addition& forest::get_data(node_ptr node)
{
	if(node->is_leaf()){
		auto r_node = std::static_pointer_cast<BPTLeaf<string, file_data_ptr> >(node);
		return r_node->data;
	} else {
		auto r_node = std::static_pointer_cast<BPTInternal<string, file_data_ptr> >(node);
		return r_node->data;
	}
}

forest::node_addition& forest::get_data(tree_t::Node* node)
{
	if(node->is_leaf()){
		auto r_node = static_cast<BPTLeaf<string, file_data_ptr>* >(node);
		return r_node->data;
	} else {
		auto r_node = static_cast<BPTInternal<string, file_data_ptr>* >(node);
		return r_node->data;
	}
}

forest::file_data_ptr forest::leaf_value(string str)
{
	return file_data_ptr(new file_data_t(str.c_str(), str.size()));
}

forest::string forest::to_string(int num)
{
	if(!num){
		return "0";
	}
	const int buf_c = 11;
	char* buf = new char[buf_c];
	int i = buf_c;
	while(num > 0){
		if(!i) throw std::out_of_range("Number is too big");
		buf[--i] = (num%10)+'0';
		num /= 10;
	}
	string ret(buf+i,buf_c-i);
	delete[] buf;
	return ret;
}

forest::string forest::read_leaf_item(file_data_ptr item)
{	
	int sz = item->size();
	char* buf = new char[sz];
	auto reader = item->get_reader();
	reader.read(buf,sz);
	string ret(buf,sz);
	delete[] buf;
	return ret;
}

void forest::opened_files_inc()
{
	std::unique_lock<std::mutex> flock(forest::opened_files_m);
	while(forest::opened_files_count > forest::OPENED_FILES_LIMIT){
		forest::opened_files_cv.wait(flock);
	}
	forest::opened_files_count++;
}

void forest::opened_files_dec()
{
	forest::opened_files_count--;
	forest::opened_files_cv.notify_all();
}
