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
#define LOG_INFO_(msg) { std::stringstream s;\
					 s << msg; \
					 MGlobal::displayInfo(s.str().c_str()); \
				   }
#define LOG_VAR(var) { std::stringstream s;\
					 s << "-- " << #var << " = " << var; \
					 MGlobal::displayInfo(s.str().c_str()); \
				   }
#define LOG_VAR2(var1, var2) { std::stringstream s;\
					 s << "-- " << #var1 << " = " << var1; \
					 s << ", " << #var2 << " = " << var2; \
					 MGlobal::displayInfo(s.str().c_str()); \
				   }
