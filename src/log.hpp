#ifndef FOREST_LOG_H
#define FOREST_LOG_H

#include <string>
#include <cassert>
#include "logger.hpp"

#ifdef DEBUG
	#define L_ERR(text) details::log_error(text)
	#define L_PUB(text) details::log_info_public(text)
	#define L_PRI(text) details::log_info_private(text)
	#define L_DET(text) details::log_info_details(text)
	#define L_EXT(text) details::log_info_extended(text)
	#define ASSERT(cond) assert(cond)
#else
	#define L_ERR(text)
	#define L_PUB(text) 
	#define L_PRI(text) 
	#define L_DET(text)
	#define L_EXT(text)
	#define ASSERT(cond)
#endif

#ifdef DEBUG_PERF
#define DP_LOG_START(p) auto p = std::chrono::high_resolution_clock::now()
#define DP_LOG_END(p,v) { auto __p_end = std::chrono::high_resolution_clock::now(); v+=std::chrono::duration_cast<std::chrono::nanoseconds>(__p_end-p).count(); }
#else
#define DP_LOG_START(p) 
#define DP_LOG_END(p,v)
#endif

namespace forest{
namespace details{
	
	extern int LOGGER_FLAG;
	extern int LOG_DETAILS;
	extern logger::Log log;
	
	void log_error(std::string str);
	void log_info_public(std::string str);
	void log_info_private(std::string str);
	void log_info_details(std::string str);
	void log_info_extended(std::string str);
	
} // details
} // forest

#endif // FOREST_LOG_H
