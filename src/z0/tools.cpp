/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cassert>
#include <stb_image_write.h>
#include "z0/libraries.h"

module z0.Tools;

import z0.Constants;
import z0.Window;

namespace z0 {

    uint32_t randomi(const uint32_t max) {
        static std::random_device rd;
        static std::uniform_int_distribution<> distr(0, static_cast<int>(max));
        std::mt19937 gen(rd());
        return static_cast<uint32_t>(distr(gen));
    }

    float randomf(const float max) {
        static std::random_device rd;
        static std::uniform_real_distribution<> distr(0, max);
        std::mt19937 gen(rd());
        return static_cast<float>(distr(gen));
    }

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

    void _stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    vector<byte> createBlankJPG() {
        vector<byte> blankJPEG;
        const auto data = new byte[1 * 1 * 3];
        data[0]   = static_cast<byte>(0);
        data[1]   = static_cast<byte>(0);
        data[2]   = static_cast<byte>(0);
        stbi_write_jpg_to_func(_stb_write_func, &blankJPEG, 1, 1, 3, data, 100);
        delete[] data;
        return blankJPEG;
    }

}

namespace std {

    string to_hexstring(const void* ptr) {
        stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<uint64_t>(ptr);
        return ss.str();
    }

    string to_string(const vec3 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "," + to_string(vec.z) + "]";
    }

    string to_string(const vec2 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "]";
    }

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

}