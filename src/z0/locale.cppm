/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Locale;

export namespace z0 {

    /**
     * Localisation helper
     */
    class Locale {
    public:
        static inline const string LOCALE_DIRECTORY = "app://locales";

        struct Translate {
            [[nodiscard]] string operator()(const string& str) const;
        };

        static void load(const string& file = "default");

        static void setLocale(const string& lang);

        static inline const string& getLocale() { return currentLocale; }

        static string getDefaultLocale();

    private:
        static inline string currentLocale{};
        static inline vector<string> loadedFileNames;
        static inline map<string, map<string, string>> translations{};
    };

    constexpr Locale::Translate _T{};


}
