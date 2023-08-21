#pragma once

#include <memory>
#include <cstdarg>

namespace ManualCAD 
{
	class Logger {
		static std::unique_ptr<Logger> LOGGER;
		static Logger& get();
	public:
		inline static void log(const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);
			get().log(fmt, args);
			va_end(args);
		}

		virtual void log(const char* fmt, va_list args) = 0;
	};

	class ConsoleLogger : public Logger {
	public:
		void log(const char* fmt, va_list args);
	};

	using LoggerImpl = ConsoleLogger;
}