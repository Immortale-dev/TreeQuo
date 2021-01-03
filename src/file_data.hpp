#ifndef FOREST_FILE_DATA_H
#define FOREST_FILE_DATA_H

#include "dbutils.hpp"

namespace forest{
namespace details{
	
	class file_data_t{
		
		using fn = std::function<void(file_data_t* self, char*, int)>;
		
		public:
			file_data_t(file_ptr file, uint_t start, uint_t length);
			file_data_t(const char* data, uint_t length);
			virtual ~file_data_t();
			uint_t size();
			void set_file(file_ptr file);
			void set_start(uint_t start);
			void set_length(uint_t length);
			void delete_cache();
			void set_cache(char* buffer);
			
			file_ptr file;
			std::mutex m,g,o;
			bool shared_lock = false;
			int c = 0;
			int res_c = 0;
			
			struct file_data_reader{
				file_data_reader(file_data_t* item);
				virtual ~file_data_reader();
				uint_t read(char* buffer, uint_t count);
				
				private:
					bool temp_cached = false;
					char* temp_cache;
					file_data_t* data;
					std::lock_guard<mutex> lock;
					uint_t pos;
			};
			file_data_reader get_reader();
			
			friend file_data_reader;
			
		private:
			uint_t start, length;
			char* data_cached;
			mutex mtx;
			bool cached = false;
	};
	
} // details
} // forest

#endif // FOREST_FILE_DATA_H
