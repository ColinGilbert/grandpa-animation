#ifndef __CONSOLE_LOGGER_H__
#define __CONSOLE_LOGGER_H__

#include "ILogger.h"

class ConsoleLogger : public grp::ILogger
{
public:
	ConsoleLogger();

	virtual void write(grp::LogType type, const wchar_t* logString);

	virtual void setLogLevel(grp::LogType level);

private:
	grp::LogType	m_logLevel;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ConsoleLogger::ConsoleLogger()
	:	m_logLevel(grp::LOG_INFO)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ConsoleLogger::write(grp::LogType type, const wchar_t* logString)
{
	if (type < m_logLevel)
	{
		return;
	}
	switch (type)
	{
	case grp::LOG_INFO:
		wprintf(L"[info] %s\r\n", logString);
		break;
	case grp::LOG_WARNING:
		wprintf(L"[warning] %s\r\n", logString);
		break;
	case grp::LOG_ERROR:
		wprintf(L"[error] %s\r\n", logString);
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ConsoleLogger::setLogLevel(grp::LogType level)
{
	m_logLevel = level;
}

#endif