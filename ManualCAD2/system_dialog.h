#pragma once

#include <initializer_list>
#include <string>

namespace ManualCAD
{
	class SystemDialog {
	public:
		struct Pattern {
			const char* extensions;
			const char* description;
		};

		enum class ButtonType {
			YesNo, Ok, OkCancel, YesNoCancel
		};

		enum class Button {
			No, YesOk, Cancel
		};

		enum class MessageBoxType {
			Warning, Question, Info, Error
		};

		static std::string open_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns);
		static std::string save_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns);
		static Button message_box(const char* title, const char* message, ButtonType buttons, MessageBoxType type);
	};
}