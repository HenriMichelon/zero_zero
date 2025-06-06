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
        /**
         * Default directory for translation files
         */
    static inline const string LOCALE_DIRECTORY = "app://locales";

    struct Translate {
        [[nodiscard]] string operator()(const string& str) const;
    };

    /**
     * Loads a translation file for the current locale
     * @param file file name
     */
    static void load(const string& file = "default");

    /**
     * Sets the current locale. Can load missing files when switching from another locale.
     * @param lang langage code
     */
    static void setLocale(const string& lang);

    /**
     * Returns the current locale
     */
    static inline const auto& getLocale() { return currentLocale; }

    /**
     * Returns the OS specific user locale
     */
    static string getDefaultLocale();

    private:
        static string currentLocale;
        static vector<string> loadedFileNames;
        static map<string, map<string, string>> translations;
    };

    /**
     * Translates a string
     */
    constexpr Locale::Translate _T{};


}
