#pragma once

#include <maya/MGlobal.h>
#include <sstream>

namespace mayaMVG {

#define LOG_ERROR(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayError(s.str().c_str()); \
				   }
#define LOG_WARNING(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayWarning(s.str().c_str()); \
				   }
#define LOG_INFO(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayInfo(s.str().c_str()); \
				   }

} // mayaMVG
