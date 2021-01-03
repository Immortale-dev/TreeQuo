#include "file_data.hpp"

forest::details::file_data_t::file_data_t(file_ptr file, uint_t start, uint_t length) : file(file), start(start), length(length) {
	// ctor
}

forest::details::file_data_t::file_data_t(const char* data, uint_t length) : start(0), length(length) { 
	data_cached = new char[length]; 
	std::memcpy(data_cached, data, length); 
	cached = true; 
}

forest::details::file_data_t::~file_data_t() { 
	delete_cache(); 
}

forest::details::uint_t forest::details::file_data_t::size(){ 
	return length; 
}

void forest::details::file_data_t::set_file(file_ptr file) { 
	this->file = file; 
}

void forest::details::file_data_t::set_start(uint_t start) { 
	this->start = start; 
}

void forest::details::file_data_t::set_length(uint_t length) { 
	this->length = length; 
}

void forest::details::file_data_t::delete_cache() { 
	if(cached){ 
		delete[] data_cached; 
		cached = false; 
	} 
}

void forest::details::file_data_t::set_cache(char* buffer) { 
	delete_cache(); 
	data_cached = new char[length]; 
	std::memcpy(data_cached, buffer, length); 
	cached = true; 
}

forest::details::file_data_t::file_data_reader forest::details::file_data_t::get_reader() { 
	return file_data_reader(this); 
}


// File data reader
forest::details::file_data_t::file_data_reader::file_data_reader(file_data_t* item) : data(item), lock(item->mtx), pos(0) { 
	if(CACHE_BYTES && data->size() <= (uint_t)CACHE_BYTES && !data->cached) {
		temp_cached = true;
		temp_cache = new char[data->size()];
	}
}

forest::details::file_data_t::file_data_reader::~file_data_reader() {
	if(temp_cached) delete[] temp_cache;
}

forest::details::uint_t forest::details::file_data_t::file_data_reader::read(char* buffer, uint_t count) { 
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
		std::memcpy(buffer, data->data_cached+pos, sz);
	}
	else{
		auto lock = data->file->get_lock();
		data->file->seekg(data->start + pos);
		data->file->read(buffer, sz);
		if(temp_cached){
			std::memcpy(temp_cache + pos, buffer, sz);
		}
	}
	pos += sz;
	return sz;
}
