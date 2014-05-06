#pragma once

#include <maya/MGlobal.h>
#include <sstream>

namespace mayaMVG {

#define CHECK(assertion) { \
						if(!(assertion)) \
							LOG_ERROR("Unexpected error (" __FILE__":" << __LINE__ << ")") \
					}

#define CHECK_RETURN(assertion) { \
						if(!(assertion)) { \
							LOG_ERROR("Unexpected error (" __FILE__":" << __LINE__<< ")") \
							return; \
						} \
					}

#define CHECK_RETURN_STATUS(assertion) { \
						if(!(assertion)) { \
							LOG_ERROR("Unexpected error (" __FILE__":" << __LINE__<< ")") \
							return MS::kFailure; \
						} \
					}

#define LOG_ERROR(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayError(s.str().c_str()); \
					 std::cerr << s.str() << std::endl; \
				   }
#define LOG_WARNING(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayWarning(s.str().c_str()); \
					 std::cerr << s.str() << std::endl; \
				   }
#define LOG_INFO(msg) { std::stringstream s;\
					 s << "[MayaMVG] " << msg; \
					 MGlobal::displayInfo(s.str().c_str()); \
					 std::cerr << s.str() << std::endl; \
				   }

} // mayaMVG
