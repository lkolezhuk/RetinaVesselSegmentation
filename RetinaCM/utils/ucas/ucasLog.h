#ifndef _UNICAS_LOG_H
#define _UNICAS_LOG_H

#include <cstdarg>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>	// C++11 only
#include <iostream>

namespace ucas
{
	enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3 };
	const debug_level DEBUG = NO_DEBUG;					//debug level of current module
	static char sys_cmd[10000];		//global variable where to store system commands 

	inline void warning(const char* message, const char* source = 0)
	{
		if(source)
			printf("\n\n**** WARNING (source: \"%s\") ****\n"
			"    |=> \"%s\"\n\n", source, message);
		else
			printf("\n\n**** WARNING ****: %s\n\n", message);
	}	

	inline void warning(const std::string & message, const char* source = 0)
	{
		if(source)
			printf("\n\n**** WARNING (source: \"%s\") ****\n"
			"    |=> \"%s\"\n\n", source, message.c_str());
		else
			printf("\n\n**** WARNING ****: %s\n\n", message.c_str());
	}

	inline void debug(debug_level dbg_level, const char* message=0, const char* source=0)
	{
		if(DEBUG >= dbg_level){
			if(message && source)
				printf("\n---- DEBUG (level %d, source: \"%s\") ----\n"
				         "    |=> \"%s\"\n\n", dbg_level, source, message);
			else if(message)
				printf("\n---- DEBUG (level %d) ----: %s\n", dbg_level, message);
			else if(source)
				printf("\n---- DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
		}
	}

	class StackPrinter
	{
		private:

			std::vector <std::string> prefixes;		// stack of prefixes 
			char paddingC;							// padding character
			int paddingL;							// padding length (default = 0)
			bool enabled;							// enable / disable printer

		public:

			StackPrinter() : paddingC(' '), paddingL(0), enabled(true){}
			void push(const std::string & str){ prefixes.push_back(str);}
			void pop(){ prefixes.pop_back();}
			void setPadding(char character, int length){ paddingC = character; paddingL = length;}
			void setEnabled(bool _enabled) {enabled = _enabled;}
			void printf(const std::string & fmt, ...)
			{
				if(enabled)
				{
					std::string prefix;
					for(size_t i=0; i<prefixes.size(); i++)
						prefix += prefixes[i];
					std::cout << padding(prefix, paddingL, paddingC);
					va_list args;
					va_start(args,fmt);
					vprintf(fmt.c_str(),args);
					va_end(args);
				}
			}
	};

	class Timer
	{
		private:

			std::chrono::time_point<std::chrono::system_clock> t0;

		public:

			Timer(){start();}

			void start(){t0 = std::chrono::system_clock::now();}
			void restart(){start();}

			template <class T> T elapsed() {
				std::chrono::duration<T> elapsed_seconds = std::chrono::system_clock::now()-t0; 
				return elapsed_seconds.count();
			}
	};

	//cross-platform current function macro
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
	# define __mcd__current__function__ __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
	# define __mcd__current__function__ __PRETTY_FUNCTION__
	#elif defined(__FUNCSIG__)
	# define __mcd__current__function__ __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	# define __mcd__current__function__ __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	# define __mcd__current__function__ __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	# define __mcd__current__function__ __func__
	#else
	# define __mcd__current__function__ "(unknown)"
	#endif
}

#endif