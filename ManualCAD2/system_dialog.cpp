#include "system_dialog.h"
#include <tinyfiledialogs.h>
#include <vector>
#include <tuple>
#include <stdexcept>

namespace ManualCAD {

    std::pair<std::vector<const char*>, std::vector<const char*>> process_patterns(const std::initializer_list<SystemDialog::Pattern>& patterns) {
        std::vector<const char*> extensions;
        extensions.reserve(patterns.size());
        std::vector<const char*> descriptions;
        descriptions.reserve(patterns.size());

        for (auto it = patterns.begin(); it != patterns.end(); ++it)
        {
            extensions.push_back(it->extensions);
            descriptions.push_back(it->description);
        }

        return { extensions, descriptions };
    }

    const char* tinyfd_buttons[] = {
        "yesno",
        "ok",
        "okcancel",
        "yesnocancel"
    };

    int tinyfd_default_buttons[] = {
        0,
        1,
        0,
        0
    };

    const char* tinyfd_types[] = {
        "warning",
        "question",
        "info",
        "error"
    };

    const char* SystemDialog::open_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns, bool allow_multiple_select)
    {
        auto vectors = process_patterns(patterns);
        return tinyfd_openFileDialog(title, "", vectors.first.size(), vectors.first.data(), "", allow_multiple_select ? 1 : 0);
    }

    const char* SystemDialog::save_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns)
    {
        auto vectors = process_patterns(patterns);
        return tinyfd_saveFileDialog(title, "", vectors.first.size(), vectors.first.data(), "");
    }

    SystemDialog::Button SystemDialog::message_box(const char* title, const char* message, ButtonType buttons, MessageBoxType type)
    {
        int result = tinyfd_messageBox(title, message, tinyfd_buttons[static_cast<int>(buttons)], tinyfd_types[static_cast<int>(type)], tinyfd_default_buttons[static_cast<int>(buttons)]);
        switch (result)
        {
        case 0:
            return buttons == ButtonType::YesNo ? Button::No : Button::Cancel;
        case 1:
            return Button::YesOk;
        case 2:
            return Button::No;
        default:
            throw std::runtime_error("Invalid value returned");
        }
    }
}
