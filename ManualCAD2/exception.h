#pragma once

#include <stdexcept>
#include <string>

#define THROW_EXCEPTION throw GlApplicationInitException()

namespace ManualCAD
{
	class GlApplicationInitException : public std::exception {
	public:
		GlApplicationInitException() : std::exception("Application init error") { }
	};

	class CommonIntersectionPointNotFoundException : public std::exception {
	public:
		CommonIntersectionPointNotFoundException() : std::exception() {}
	};

	class TimeoutException : public std::exception {
	public:
		TimeoutException() : std::exception() {}
	};
}