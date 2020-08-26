#ifndef FOREST_LOG_H
#define FOREST_LOG_H

#include <string>
//#include "logger.hpp"

namespace forest{
	
	extern int LOGGER_FLAG;
	extern int LOG_DETAILS;
	//extern logger::Log log;
	
	void log_error(std::string str);
	void log_info_public(std::string str);
	void log_info_private(std::string str);
	void log_info_details(std::string str);
	void log_info_extended(std::string str);
	
}

#endif // FOREST_LOG_H
