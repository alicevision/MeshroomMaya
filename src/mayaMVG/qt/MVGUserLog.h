#ifndef MVGUSERLOG_H
#define	MVGUSERLOG_H

#include <sstream>
#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/core/MVGLog.h"


namespace mayaMVG {
	
#define USER_ERROR(msg) { std::stringstream s;\
					s << "[ERROR] " << msg; \
					MVGProjectWrapper::instance().appendLogText(QString(s.str().c_str()));\
					LOG_ERROR(msg);\
				   }
#define USER_WARNING(msg) { std::stringstream s;\
					s << "[WARNING] " << msg; \
					MVGProjectWrapper::instance().appendLogText(QString(s.str().c_str()));\
					LOG_WARNING(msg);\
				   }
#define USER_INFO(msg) { std::stringstream s;\
					s << "[INFO] " << msg; \
					MVGProjectWrapper::instance().appendLogText(QString(s.str().c_str()));\
					LOG_INFO(msg);\
				   }
}


#endif