#ifndef FOREST_DBUTILS_H
#define FOREST_DBUTILS_H

#include <functional>
#include <memory>
#include <string>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <iostream> //for debug
#include "dbfs.hpp"
#include "bplustree.hpp"
#include "dbdriver.hpp"
#include "dbexception.hpp"
#include "log.hpp"


namespace forest{
	
	extern int CACHE_BYTES;
	
	enum class TREE_TYPES { KEY_STRING };
	enum class RECORD_POSITION{ BEGIN, END, LOWER, UPPER };
	enum class KEY_TYPES { STRING };
	enum class NODE_TYPES { INTR, LEAF };
	enum class SAVE_TYPES{ LEAF, INTR, BASE };
	
	class Tree;
	class file_data_t;
	
	using string = std::string;
	using int_t = long long int;
	using uint_t = unsigned long long int;
	using file_pos_t = unsigned long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	using int_a = std::atomic<int>;
	using uintptr_t = std::uintptr_t;
	
	using file_data_ptr = std::shared_ptr<file_data_t>;
	
	using driver_t = DBDriver<string, file_data_ptr>;
	using tree_t = BPlusTree<string, file_data_ptr, Tree>;
	
	using tree_ptr = std::shared_ptr<Tree>;
	using node_ptr = tree_t::node_ptr;
	
	using child_lengths_vec_ptr = std::vector<uint_t>*;
	using child_keys_vec_ptr = std::vector<tree_t::key_type>*;
	using child_values_vec_ptr = std::vector<tree_t::val_type>*;
	using child_nodes_vec_ptr = std::vector<string>*;
	
	struct tree_leaf_read_t {
		child_keys_vec_ptr child_keys;
		child_lengths_vec_ptr child_lengths;
		uint_t start_data;
		DBFS::File* file;
		string left_leaf, right_leaf;
	};
	struct tree_intr_read_t {
		NODE_TYPES childs_type;
		child_keys_vec_ptr child_keys;
		child_nodes_vec_ptr child_values;
	};
	struct tree_base_read_t {
		TREE_TYPES type;
		NODE_TYPES branch_type;
		uint_t count;
		int factor;
		string branch;
	};
	
	////////////////////////////////////////////////////////////////
	
	inline node_addition& get_data(node_ptr node)
	{
		if(node->is_leaf()){
			auto r_node = std::static_pointer_cast<BPTLeaf<string, file_data_ptr> >(node);
			return r_node->data;
		} else {
			auto r_node = std::static_pointer_cast<BPTInternal<string, file_data_ptr> >(node);
			return r_node->data;
		}
	}
	
	inline node_addition& get_data(tree_t::Node* node)
	{
		if(node->is_leaf()){
			auto r_node = static_cast<BPTLeaf<string, file_data_ptr>* >(node);
			return r_node->data;
		} else {
			auto r_node = static_cast<BPTInternal<string, file_data_ptr>* >(node);
			return r_node->data;
		}
	}
	
	class file_data_t{
		
		using fn = std::function<void(file_data_t* self, char*, int)>;
		using file_ptr = std::shared_ptr<DBFS::File>;
		
		public:
			file_data_t(file_ptr file, uint_t start, uint_t length) : file(file), start(start), length(length) { };
			file_data_t(const char* data, uint_t length) : start(0), length(length) { data_cached = new char[length]; memcpy(data_cached, data, length); cached = true; };
			virtual ~file_data_t() { /*std::cout << "-----DESTRUCT_file_data_t " + file->name() + " " + std::to_string((uint_t)this) + "\n";*/ delete_cache(); };
			uint_t size(){ return length; };
			void set_file(file_ptr file) { this->file = file; };
			void set_start(uint_t start) { this->start = start; };
			void set_length(uint_t length) { this->length = length; };
			void delete_cache() { if(cached){ delete[] data_cached; cached = false; } };
			void set_cache(char* buffer) { delete_cache(); data_cached = new char[length]; memcpy(data_cached, buffer, length); cached = true; };
			
			file_ptr file;
			std::mutex m,g,o;
			bool shared_lock = false;
			int c = 0;
			int res_c = 0;
			
			struct file_data_reader{
				file_data_reader(file_data_t* item) : data(item), lock(item->mtx), pos(0) { 
					if(CACHE_BYTES && data->size() <= (uint_t)CACHE_BYTES && !data->cached) {
						temp_cached = true;
						temp_cache = new char[data->size()];
					}
				};
				virtual ~file_data_reader() {
					if(temp_cached) delete[] temp_cache;
				};
				uint_t read(char* buffer, uint_t count) { 
					uint_t sz = std::min(data->size()-pos, count);
					if(!sz){
						// Save cache
						if(temp_cached){
							data->data_cached = temp_cache;
							data->cached = true;
							temp_cached = false;
							temp_cache = nullptr;
						}
						return sz;
					}
					if(data->cached){
						memcpy(buffer, data->data_cached+pos, sz);
					}
					else{
						auto lock = data->file->get_lock();
						//std::cout << "FN: " + data->file->name() + "\n";
						data->file->seekg(data->start + pos);
						data->file->read(buffer, sz);
						if(temp_cached){
							memcpy(temp_cache + pos, buffer, sz);
						}
					}
					pos += sz;
					return sz;
				};
				
				private:
					bool temp_cached = false;
					char* temp_cache;
					file_data_t* data;
					std::lock_guard<mutex> lock;
					uint_t pos;
			};
			file_data_reader get_reader() { return file_data_reader(this); };
			
			friend file_data_reader;
			
		private:
			
			uint_t start, length;
			char* data_cached;
			mutex mtx;
			bool cached = false;
	};
	
	inline file_data_ptr leaf_value(string str)
	{
		return file_data_ptr(new file_data_t(str.c_str(), str.size()));
	}
	
	inline string to_string(int num)
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
}

#endif //FOREST_DBUTILS_H
