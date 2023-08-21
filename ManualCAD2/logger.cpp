#include "logger.h"
#include <cstdio>

namespace ManualCAD
{
	std::unique_ptr<Logger> Logger::LOGGER = nullptr;

	void ConsoleLogger::log(const char* fmt, va_list args)
	{
		vprintf(fmt, args);
	}

	Logger& Logger::get()
	{
		if (LOGGER == nullptr)
			LOGGER = std::make_unique<LoggerImpl>();
		return *LOGGER;
	}

}