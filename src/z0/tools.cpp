#include "z0/z0.h"

namespace z0 {

    uint32_t randomi(uint32_t max) {
        static std::random_device rd;
        static std::uniform_int_distribution<> distr(0, static_cast<int>(max));

        std::mt19937 gen(rd());
        return static_cast<uint32_t>(distr(gen));
    }

    float randomf(float max) {
        static std::random_device rd;
        static std::uniform_real_distribution<> distr(0, max);

        std::mt19937 gen(rd());
        return static_cast<float>(distr(gen));
    }

    vector<string_view> split(string_view str, char delimiter) {
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

/*
    JPH::Mat44 glmToJolt(const mat4& glmMat) {
        JPH::Mat44 joltMat;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                joltMat(row, col) = glmMat[col][row];  // Transpose the matrix
            }
        }
        return joltMat;
    }*/


}

namespace std {
    
    string to_string(vec3 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "," + to_string(vec.z) + "]";
    }
    string to_string(vec2 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "]";
    }
    string to_string(vec4 vec) {
        return "[" + to_string(vec.x) + "," + to_string(vec.y) + "," + to_string(vec.z) + to_string(vec.w) + "]";
    }

    vec3 to_vec3(const string& str) {
        stringstream ss(str);
        float x, y, z;
        char comma;
        ss >> x >> comma >> y >> comma >> z;
        return vec3{x, y, z};
    }
}