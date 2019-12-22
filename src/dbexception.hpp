#ifndef DBEXCEPTION_H
#define DBEXCEPTION_H

#include <string>

class DBException : public std::exception {
    
    public:
		DBException(ERRORS code)
		{
			msg = error_messages[code];
		}
		const char * what () const throw ()
		{
			return msg.c_str();
		}
		
		enum class ERRORS{
			CANNOT_CREATE_ROOT
		}
		
	private:
		std::string msg;
		std::string error_messages[] = {
			"Cannot create root files"
		}
			   
}

#endif //DBEXCEPTION_H

