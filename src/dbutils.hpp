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
#include "dbfs.hpp"
#include "bplustree.hpp"
#include "dbdriver.hpp"
#include "dbexception.hpp"

namespace forest{
	
	using string = std::string;
	using int_t = long long int;
	using uint_t = unsigned long long int;
	using file_pos_t = unsigned long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	using int_a = std::atomic<int>;
	
	enum class TREE_TYPES { KEY_STRING };
	class file_data_t;
	
	using file_data_ptr = std::shared_ptr<file_data_t>;
	
	using driver_t = DBDriver<string, file_data_ptr>;
	using tree_t = BPlusTree<string, file_data_ptr, driver_t>;

	////////////////////////////////////////////////////////////////
	/*
	class file_data_t{
		
		using int_t = long long int;
		using fn = std::function<void(file_data_t* self, char*, int)>;
		using file_ptr = std::shared_ptr<DBFS::File>;
		
		public:
			file_data_t(int_t start, int_t length, file_ptr file, fn read) : start(start), length(length), _read(read) { this->file = file; reset(); };
			file_data_t(int_t length, fn read) : length(length), _read(read) { start = 0; reset(); };
			virtual ~file_data_t(){ };
			int_t size()
			{
				return length;
			};
			void reset(){
				curr = start;
			};
			int_t read(char* data, int count)
			{ 
				int_t sz = length+(start-curr);
				if(!sz) return 0;
				int c = std::min(sz,(int_t)count);
				_read(this, data, c);
				curr += c;
				return c;
			};
			int_t start, length, curr;
			fn _read;
			file_ptr file;
			
			static file_data_t from_string(string str){
				int* i = new int(0);
				return file_data_t(str.size(), [str,i](file_data_t* self, char* buf, int count){
					for(;*i<count;(*i)++){
						buf[*i] = str[*i];
					}
				});
			}
	};
	*/
	class file_data_t{
		
		using fn = std::function<void(file_data_t* self, char*, int)>;
		using file_ptr = std::shared_ptr<DBFS::File>;
		
		public:
			file_data_t(file_ptr file, uint_t start, uint_t length) : file(file), start(start), length(length) { };
			file_data_t(const char* data, uint_t length) : start(0), length(length) { data_cached = new char[length]; memcpy(data_cached, data, length); cached = true; };
			virtual ~file_data_t() { delete_cache(); };
			uint_t size(){ return length; };
			void set_file(file_ptr file) { this->file=file; };
			void set_start(uint_t start) { this->start = start; };
			void set_length(uint_t length) { this->length = length; };
			void delete_cache() { if(cached) delete[] data_cached; cached = false; };
			file_ptr file;
			
			struct file_data_reader{
				file_data_reader(file_data_t* item) : data(item), lock(item->mtx), pos(0) { };
				virtual ~file_data_reader() { };
				uint_t read(char* buffer, uint_t count) { 
					uint_t sz = std::min(data->size()-pos, count);
					if(!sz)
						return sz;
					if(data->cached){
						memcpy(buffer, data->data_cached+pos, sz);
					}
					else{
						auto lock = data->file->get_lock();
						data->file->seek(data->start + pos);
						data->file->read(buffer, sz);
					}
					pos += sz;
					return sz;
				};
				
				private:
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
	

}

#endif //FOREST_DBUTILS_H
