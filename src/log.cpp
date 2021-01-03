#include "log.hpp"

namespace forest{
namespace details{

	/**
	 * 0b - L_ERROR
	 * 1b - L_WARNING
	 * 2b - L_INFO
	 */
	int LOGGER_FLAG = 0b011;
	
	/**
	 * 0b - Public members
	 * 1b - Private members
	 * 2b - Detailed Logs
	 * 3b - Custom details
	 */
	int LOG_DETAILS = 0b0111;
	
	std::string LOG_PATH = "./logs/forest.log";
	
	logger::Log log(LOG_PATH, LOGGER_FLAG);
	
} // details
} // forest

void forest::details::log_error(std::string str)
{
	log.error(str);
}

void forest::details::log_info_public(std::string str)
{
	if(LOG_DETAILS & 1){
		log.info(str);
	}
}

void forest::details::log_info_private(std::string str)
{
	if(LOG_DETAILS & 2){
		log.info(str);
	}
}

void forest::details::log_info_details(std::string str)
{
	if(LOG_DETAILS & 4){
		log.info(str);
	}
}

void forest::details::log_info_extended(std::string str)
{
	if(LOG_DETAILS & 8){
		log.info(str);
	}
}
