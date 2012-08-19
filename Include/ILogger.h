#ifndef __GRP_I_LOGGER_H__
#define __GRP_I_LOGGER_H__

namespace grp
{

enum LogType
{
	LOG_INFO = 0,
	LOG_WARNING,
	LOG_ERROR
};

class ILogger
{
public:
	virtual ~ILogger(){}

	virtual void write(LogType type, const Char* logString) = 0;

	virtual void setLogLevel(LogType level) = 0;
};

}

#endif
