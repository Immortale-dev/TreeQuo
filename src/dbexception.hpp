#ifndef DBEXCEPTION_H
#define DBEXCEPTION_H

#include <string>

class DBException : public std::exception {
    
    public:
		enum class ERRORS{
			CANNOT_CREATE_ROOT,
			CANNOT_CREATE_FILE,
			NOT_VALID_TREE_TYPE,
			TREE_DOES_NOT_EXISTS,
			TREE_ALREADY_EXISTS
		};
		
		DBException(ERRORS code)
		{
			msg = error_messages[(int)code];
		}
		const char * what () const throw ()
		{
			return msg.c_str();
		}
		
	private:
		std::string msg;
		const std::string error_messages[5] = {
			"Cannot create root files",
			"Cannot create files",
			"Not valid tree type",
			"Trying to open not valid tree type",
			"Tree with name you trying to create is already exists"
		};
			   
};

#endif //DBEXCEPTION_H

