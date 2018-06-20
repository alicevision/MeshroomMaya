#pragma once

#include "meshroomMaya/core/MVGLog.hpp"
#include <sstream>

namespace meshroomMaya
{

#define USER_ERROR(msg)                                                                            \
    {                                                                                              \
        std::stringstream s;                                                                       \
        s << "[ERROR] " << msg;                                                                    \
        LOG_ERROR(msg);                                                                            \
    }

#define USER_WARNING(msg)                                                                          \
    {                                                                                              \
        std::stringstream s;                                                                       \
        s << "[WARNING] " << msg;                                                                  \
        LOG_WARNING(msg);                                                                          \
    }

#define USER_INFO(msg)                                                                             \
    {                                                                                              \
        std::stringstream s;                                                                       \
        s << "[INFO] " << msg;                                                                     \
        LOG_INFO(msg);                                                                             \
    }

} // namespace
