#include "logger.h"
#include <cstdio>
#include <Windows.h>

namespace 
{
	void change_windows_console_color(WORD color_code)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, color_code);
	}
}

namespace ManualCAD
{
	class WindowsConsoleLogger : public Logger {
	public:
		void log(const char* fmt, va_list args) override
		{
			change_windows_console_color(15);
			vprintf(fmt, args);
		}
		void log_error(const char* fmt, va_list args) override
		{
			change_windows_console_color(12);
			vprintf(fmt, args);
		}
		void log_warning(const char* fmt, va_list args) override
		{
			change_windows_console_color(14);
			vprintf(fmt, args);
		}
	};

	class FastFileLogger : public Logger {
		const char* logfile = "cad.log";
		FILE* file;
	public:
		FastFileLogger()
		{
			if (fopen_s(&file, logfile, "a") != 0)
			{
				printf("Failed to open log file!\n");
				file = nullptr;
			}
		}
		void log(const char* fmt, va_list args) override
		{
			if (file != nullptr)
			{
				vfprintf(file, fmt, args);
			}
		}
		void log_error(const char* fmt, va_list args) override { log(fmt, args); }
		void log_warning(const char* fmt, va_list args) override { log(fmt, args); }
	};

	class FileLogger : public Logger {
		const char* logfile = "cad.log";
		FILE* file;
	public:
		void log(const char* fmt, va_list args) override
		{
			if (fopen_s(&file, logfile, "a") == 0)
			{
				vfprintf(file, fmt, args);
				fclose(file);
			}
		}
		void log_error(const char* fmt, va_list args) override { log(fmt, args); }
		void log_warning(const char* fmt, va_list args) override { log(fmt, args); }
	};

	template <class... Ls>
	class CompositeLogger : public Logger {
		std::tuple<Ls...> loggers;
	public:
		void log(const char* fmt, va_list args) override { std::apply([fmt, args](Ls&... ls) { (ls.log(fmt, args), ...); }, loggers); }
		void log_error(const char* fmt, va_list args) override { std::apply([fmt, args](Ls&... ls) { (ls.log_error(fmt, args), ...); }, loggers); }
		void log_warning(const char* fmt, va_list args) override { std::apply([fmt, args](Ls&... ls) { (ls.log_warning(fmt, args), ...); }, loggers); }
	};

	//using LoggerImpl = CompositeLogger<WindowsConsoleLogger, FileLogger>;
	using LoggerImpl = WindowsConsoleLogger;

	std::unique_ptr<Logger> Logger::LOGGER = std::make_unique<LoggerImpl>();
}