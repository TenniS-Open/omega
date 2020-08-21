//
// Created by kier on 2020/8/18.
//

#ifndef OMEGA_ANDROID_H
#define OMEGA_ANDROID_H

#include "../platform.h"

#if OHM_PLATFORM_OS_ANDROID

#include "../loglevel.h"

#include "android/log.h"
#define ANDROID_LOG(_LEVEL, _TAG, ...) __android_log_print(_LEVEL, _TAG, __VA_ARGS__)

namespace ohm {
    namespace android {
        inline android_LogPriority __android_log_level(LogLevel level) {
            switch (level) {
                default: return ANDROID_LOG_UNKNOWN;
                case LOG_NONE: return ANDROID_LOG_VERBOSE;
                case LOG_DEBUG: return ANDROID_LOG_DEBUG;
                case LOG_INFO: return ANDROID_LOG_INFO;
                case LOG_WARNING: return ANDROID_LOG_WARN;
                case LOG_ERROR: return ANDROID_LOG_ERROR;
                case LOG_FATAL: return ANDROID_LOG_FATAL;
            }
        }

        inline void log(LogLevel level, const std::string &tag, const std::string &msg) {
            ANDROID_LOG(__android_log_level(level), tag.c_str(), "%s", msg.c_str());
        }
    }
}

#endif

#endif //OMEGA_ANDROID_H
