#include "system_dialog.h"
#include <tinyfiledialogs.h>
#include <nfd.h>
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

    std::string link_patterns(const std::vector<const char*> extensions) {
        if (extensions.empty()) 
            return "";

        std::string result(extensions[0]);
        for (int i = 1; i < extensions.size(); ++i)
            result += ";" + std::string{ extensions[i] };

        return result;
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

    std::string SystemDialog::open_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns)
    {
        nfdchar_t* outPath = NULL;
        std::string pattern_str = link_patterns(process_patterns(patterns).first);
        nfdresult_t result = NFD_OpenDialog(pattern_str.c_str(), NULL, &outPath);

        if (result == NFD_OKAY) {
            std::string ret(outPath);
            free(outPath);
            return ret;
        }
        else if (result == NFD_CANCEL) {
            return "";
        }
        else {
            throw std::runtime_error(NFD_GetError());
        }

        // tinyfiledialog implementation -- sometimes throws on system DLLs
        /*auto vectors = process_patterns(patterns);
        return tinyfd_openFileDialog(title, "", vectors.first.size(), vectors.first.data(), "", allow_multiple_select ? 1 : 0);*/
    }

    std::string SystemDialog::save_file_dialog(const char* title, const std::initializer_list<Pattern>& patterns)
    {
        nfdchar_t* outPath = NULL;
        std::string pattern_str = link_patterns(process_patterns(patterns).first);
        nfdresult_t result = NFD_SaveDialog(pattern_str.c_str(), NULL, &outPath);

        if (result == NFD_OKAY) {
            std::string ret(outPath);
            free(outPath);
            return ret;
        }
        else if (result == NFD_CANCEL) {
            return "";
        }
        else {
            throw std::runtime_error(NFD_GetError());
        }

        // tinyfiledialog implementation -- sometimes throws on system DLLs
        /*auto vectors = process_patterns(patterns);
        return tinyfd_saveFileDialog(title, "", vectors.first.size(), vectors.first.data(), "");*/
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
