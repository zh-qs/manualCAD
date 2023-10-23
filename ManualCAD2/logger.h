#pragma once

#include <memory>
#include <cstdarg>
#include <cstdio>
#include <tuple>

namespace ManualCAD 
{
	class Logger {
		static std::unique_ptr<Logger> LOGGER;
	public:
		inline static void log(const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);
			LOGGER->log(fmt, args);
			va_end(args);
		}
		inline static void log_error(const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);
			LOGGER->log_error(fmt, args);
			va_end(args);
		}
		inline static void log_warning(const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);
			LOGGER->log_warning(fmt, args);
			va_end(args);
		}

		virtual void log(const char* fmt, va_list args) = 0;
		virtual void log_error(const char* fmt, va_list args) = 0;
		virtual void log_warning(const char* fmt, va_list args) = 0;
	};
}