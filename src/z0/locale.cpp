/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <json.hpp>
#include "z0/libraries.h"

module z0.Locale;

import z0.Tools;
import z0.VirtualFS;

namespace z0 {

    string Locale::Translate::operator()(const string& str) const {
        return translations[currentLocale].contains(str) ? string(translations[currentLocale][str]) : str;
    }

    string Locale::getDefaultLocale() {
#ifdef _WIN32
        WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
        if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH)) {
            const auto locale = wstring_to_string(wstring(localeName));
            const auto pos = locale.find('-');
            if (pos != string::npos) {
                return locale.substr(0, pos);
            }
            return locale;
        }
#endif
        return "en";
    }

    void Locale::setLocale(const string& lang) {
        currentLocale = lang;
        // load missing translation files
        for (const auto& file : loadedFileNames) {
            if (!translations[currentLocale].contains(file)) {
                load(file);
            }
        }
    }

    void Locale::load(const string &file) {
        if (currentLocale.empty()) {
            currentLocale = getDefaultLocale();
        }
        const auto newLocale = nlohmann::json::parse(VirtualFS::openReadStream(LOCALE_DIRECTORY + "/" + file + "_" + currentLocale + ".json"));
        translations[currentLocale].merge(newLocale.get<map<string, string>>());
        loadedFileNames.push_back(file);
    }


}
