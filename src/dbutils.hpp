#ifndef FOREST_DBUTILS_H
#define FOREST_DBUTILS_H

#include <functional>
#include <memory>
#include <string>
#include <algorithm>
#include <mutex>
#include <atomic>
#include "dbfs.hpp"
#include "bplustree.hpp"
#include "dbdriver.hpp"

namespace forest{
	
	using string = std::string;
	using int_t = long long int;
	using file_pos_t = unsigned long long int;
	using mutex = std::mutex;
	using void_shared = std::shared_ptr<void>;
	using int_a = std::atomic<int>;
	
	enum class TREE_TYPES { KEY_STRING };
	class file_data_t;
	
	using driver_t = DBDriver<string, file_data_t>;
	using tree_t = BPlusTree<string, file_data_t, driver_t>;

	////////////////////////////////////////////////////////////////

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
	};

}

#endif //FOREST_DBUTILS_H
