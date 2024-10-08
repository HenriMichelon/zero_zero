module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cassert>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Window;

namespace z0 {

    mat4 perspective(float fovRadians, float nearDistance, float farDistance) {
        const auto aspect = Application::get()._getDevice().getAspectRatio();
        assert(glm::abs(aspect - numeric_limits<float>::epsilon()) > 0.0f);
        const auto tanHalfFovy = tan(fovRadians / 2.f);
        auto projectionMatrix       = mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = farDistance / (farDistance - nearDistance);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(farDistance * nearDistance) / (farDistance - nearDistance);
        return projectionMatrix;
    }

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

}

namespace std {

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