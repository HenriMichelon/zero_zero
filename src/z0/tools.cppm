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
#include "z0/libraries.h"

export module z0.Tools;

import z0.Constants;
import z0.Window;

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
                   ENGINE_NAME,
                   MB_OK | MB_ICONINFORMATION);
#endif
#if defined(_DEBUG) && defined(_WIN32)
        __debugbreak();
#else
        throw runtime_error(stringstream.str());
#endif
    }

    /**
     * Log a message into the logging system. 
     * Log messages can be deferred when displayed inside a separate Window if emitted from a thread different from the main thread.
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
    uint32_t randomi( uint32_t max);

    /**
    * Returns a random value in the range [0.0f, max]
    */
    float randomf( float max);

    /**
     * Split a string
     */
    vector<string_view> split( string_view str,  char delimiter);

    int numMipmapLevels(const int width, const int height);

    vector<byte> createBlankJPG();

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

    inline float lerp(const float a, const float b, const float t) {
        return std::lerp(a, b, t);
    }


}

export namespace std {

    /**
     * Helper to log a memory address in hexadecimal
     */
    string to_hexstring(const void* ptr);
    string to_hexstring(const uint32_t ptr);

    /**
     * Helper to log a vec3 (std lib code convention)
     */
    string to_string(vec3 vec);

    /**
     * Helper to log a vec2 (std lib code convention)
     */
    string to_string(vec2 vec);

    /**
     * Helper to log a vec4 (std lib code convention)
     */
    string to_string(vec4 vec);

    string to_string(quat vec);

    string to_lower(const string &str);

    /**
     * Helper to convert a vec3 from a string (std lib code convention)
     */
    vec3 to_vec3(const string &str);

    /**
     * Helper to convert a vec4 from a string (std lib code convention)
     */
    vec4 to_vec4(const string &str);

    /**
     * Custom hash function for vec2 (std lib code convention)
     */
    template <>
    struct hash<vec2> {
        std::size_t operator()(const vec2 &v) const {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
        }
    };

    /**
     * Custom hash function for vec3 (std lib code convention)
     */
    template <>
    struct hash<vec3> {
        std::size_t operator()(const vec3 &v) const {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1) ^ (std::hash<float>()(v.z) << 2);
        }
    };

    /**
     * Custom hash function for vec4 (std lib code convention)
     */
    template <>
    struct hash<vec4> {
        std::size_t operator()(const vec4 &v) const {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1) ^ (std::hash<float>()(v.z) << 2) ^ (
                std::hash<float>()(v.w) << 3);
        }
    };

    inline string wstring_to_string(const std::wstring &wstr) {
        wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(wstr);
    }

    inline wstring string_to_wstring(const std::string &str) {
        wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
    }
}
