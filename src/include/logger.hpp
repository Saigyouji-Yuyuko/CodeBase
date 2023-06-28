#pragma once

namespace CodeBase {

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

#define LOGTRACE(logLevel, context, ...)
#define LOGDEBUG(logLevel, context, ...)
#define LOGINFO(logLevel, context, ...)
#define LOGWARN(logLevel, context, ...)
#define LOGERROR(logLevel, context, ...)
#define LOGFATAL(logLevel, context, ...)

}// namespace CodeBase