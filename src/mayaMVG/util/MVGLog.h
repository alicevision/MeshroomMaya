#pragma once

#include <maya/MGlobal.h>
#include <sstream>

#define LOG_ERROR(func, msg) { std::stringstream s;\
					 s << "[" << func << "] " << msg; \
					 MGlobal::displayError(s.str().c_str()); \
				   }
#define LOG_WARNING(func, msg) { std::stringstream s;\
					 s << "[" << func << "] " << msg; \
					 MGlobal::displayWarning(s.str().c_str()); \
				   }
#define LOG_INFO(func, msg) { std::stringstream s;\
					 s << "[" << func << "] " << msg; \
					 MGlobal::displayInfo(s.str().c_str()); \
				   }
