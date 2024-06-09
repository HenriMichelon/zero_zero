#pragma once

namespace z0 {

    void die(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (auto v : initializer_list<string_view>{ s... }) {
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

    void log(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (auto v : initializer_list<string_view>{ s... }) {
            stringstream << v << " ";
        }
#ifdef _WIN32
       Window::_log(stringstream.str());
#endif
    }

    string toString(vec3 vec);
    //JPH::Mat44 glmToJolt(const mat4& glmMat);

    vector<string_view> split(string_view str, char delimiter);

    // returns a random value in the range [0, max]
    uint32_t randomi(uint32_t max = 100);
    // returns a random value in the range [0.0f, max]
    float randomf(float max = 100.0f);

}
