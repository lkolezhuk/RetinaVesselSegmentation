#ifndef _UNICAS_EXCEPTIONS_H
#define _UNICAS_EXCEPTIONS_H

#include <string>
#include "ucasStringUtils.h"

namespace ucas
{
	enum mtype { WARNING_MSG, ERROR_MSG };

	class Error
	{
		protected:

			std::string message;
			mtype message_type;

		public:

			Error(void);
			Error(const std::string & new_message, mtype _message_type = ERROR_MSG)
			{
				message = new_message;
				message_type = _message_type;
			}
			virtual ~Error(void){}
			const char* what() const{return message.c_str();}
			mtype getType() {return message_type;}
	};

	class CannotCreateFolderError : public Error
	{
		public: 
			
			CannotCreateFolderError(const std::string & path) : Error(strprintf("Cannot create folder at \"%s\"", path.c_str()), ERROR_MSG){}
	};
	
	class CannotOpenFileError : public Error
	{
		public: 
			
			CannotOpenFileError(const std::string & path) : Error(strprintf("Cannot open file at \"%s\"", path.c_str()), ERROR_MSG){}
	};

	class FileNotExistsError : public Error
	{
		public: 

			FileNotExistsError(const std::string & path) : Error(strprintf("File does not exists at \"%s\"", path.c_str()), ERROR_MSG){}
	};

	#ifndef UCAS_THROW
	#define UCAS_THROW(m) ( throw ucas::Error(std::string("in ") + __mcd__current__function__ + ": " + (m)) )
	#endif
}

#endif