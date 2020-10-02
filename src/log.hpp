#ifndef FOREST_LOG_H
#define FOREST_LOG_H

#include <string>
#include "logger.hpp"
#include "dbutils.hpp"

#ifdef DEBUG
	#define L_ERR(text) log_error(text)
	#define L_PUB(text) log_info_public(text)
	#define L_PRI(text) log_info_private(text)
	#define L_DET(text) log_info_details(text)
	#define L_EXT(text) log_info_extended(text)
#else
	#define L_ERR(text)
	#define L_PUB(text) 
	#define L_PRI(text) 
	#define L_DET(text)
	#define L_EXT(text)
#endif

namespace forest{
	
	extern int LOGGER_FLAG;
	extern int LOG_DETAILS;
	extern logger::Log log;
	
	void log_error(std::string str);
	void log_info_public(std::string str);
	void log_info_private(std::string str);
	void log_info_details(std::string str);
	void log_info_extended(std::string str);
	
}

#endif // FOREST_LOG_H
