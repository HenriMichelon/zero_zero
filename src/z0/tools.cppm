module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "z0/libraries.h"

export module Z0:Tools;

import :Constants;
import :Window;

export namespace z0 {
    /**
     * Violently stop the application, used if something goes wrong
     */
    void die(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (const auto v : initializer_list<string_view>{ s... }) {
            stringstream << v << " ";
        }
#ifdef _WIN32
        MessageBox(nullptr,
                   stringstream.str().c_str(),
                   ENGINE_NAME.c_str(),
                   MB_OK | MB_ICONINFORMATION);
#endif
        throw runtime_error(stringstream.str());
    }

    /**
     * Log a message into the logging system. 
     * Log messages can be deferred when displayed inside a separate window if emited from a thread different from the main thread.
     */
#ifdef DISABLE_LOG
    void log(convertible_to<string_view> auto&& ...s) {}
#else
    void log(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (const auto v : initializer_list<string_view>{ s... }) {
            stringstream << v << " ";
        }
#ifdef _WIN32
        Window::_log(stringstream.str());
#endif
    }
#endif

    /**
    * Returns a random value in the range [0, max]
    */
    uint32_t randomi(const uint32_t max) {
        static std::random_device rd;
        static std::uniform_int_distribution<> distr(0, static_cast<int>(max));
        std::mt19937 gen(rd());
        return static_cast<uint32_t>(distr(gen));
    }

    /**
    * Returns a random value in the range [0.0f, max]
    */
    float randomf(const float max) {
        static std::random_device rd;
        static std::uniform_real_distribution<> distr(0, max);
        std::mt19937 gen(rd());
        return static_cast<float>(distr(gen));
    }


    /**
     * Split a string
     */
    vector<string_view> split(const string_view str, const char delimiter) {
        vector<std::string_view> result;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string_view::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        result.push_back(str.substr(start)); // Add the last token
        return result;
    }

    /**
     * lerp for a vec2 using std::lerp for componants
    */
    inline vec2 lerp(const vec2 a, const vec2 b, const float t) {
        return vec2{
            std::lerp(a.x, b.x, t),
            std::lerp(a.y, b.y, t),
        };
    }

    /**
     * lerp for a vec3 using std::lerp for componants
    */
    inline vec3 lerp(const vec3 a, const vec3 b, const float t) {
        return vec3{
            std::lerp(a.x, b.x, t),
            std::lerp(a.y, b.y, t),
            std::lerp(a.z, b.z, t),
        };
    }

    /**
     * lerp for a vec4 using std::lerp for componants
    */
    inline vec4 lerp(const vec4 a, const vec4 b, const float t) {
        return vec4{
            std::lerp(a.x, b.x, t),
            std::lerp(a.y, b.y, t),
            std::lerp(a.z, b.z, t),
            std::lerp(a.w, b.w, t),
        };
    }

}

export namespace std {

    /**
     * Helper to log a vec3 (std lib code convention)
     */
    string to_string(const vec3 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "," + to_string(vec.z) + "]";
    }

    /**
     * Helper to log a vec2 (std lib code convention)
     */
    string to_string(const vec2 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "]";
    }

    /**
     * Helper to log a vec4 (std lib code convention)
     */
    string to_string(const vec4 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "," + to_string(vec.z) + to_string(vec.w) + "]";
    }

    string to_lower(const string& str) {
        auto s = str;
        // https://en.cppreference.com/w/cpp/string/byte/tolower
         std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::tolower(c); }
                  );
        return s;
    }

    vec3 to_vec3(const string& str) {
        stringstream ss(str);
        vec3 result{};
        if (string token; getline(ss, token, ',')) {
            result.x = stof(token);
            if (getline(ss, token, ',')) {
                result.y = stof(token);
                if (getline(ss, token, ',')) {
                    result.z = stof(token);
                }
            }
        }
        return result;
    }
    
    vec4 to_vec4(const string& str) {
        stringstream ss(str);
        vec4 result{};
         if (string token; getline(ss, token, ',')) {
            result.x = stof(token);
            if (getline(ss, token, ',')) {
                result.y = stof(token);
                if (getline(ss, token, ',')) {
                    result.z = stof(token);
                    if (getline(ss, token, ',')) {
                        result.w = stof(token);
                    }
                }
            }
        }
        return result;
    }

    inline string wstring_to_string(const std::wstring& wstr) {
        wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(wstr);
    }

    inline wstring string_to_wstring(const std::string& str) {
        wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
    }
}