#pragma once

namespace z0 {

    /**
    * @file tools.h
    * @brief Some useful global functions
    */

    /**
     * Violently stop the application, used is something goes wrong
     */
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

    /**
     * Log a message into the logging system. 
     * Log messages can be deferred when displayed inside a separate window if emited from a thread different from the main thread.
     */
#ifdef DISABLE_LOG
    void log(convertible_to<string_view> auto&& ...s) {}
#else
    void log(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (auto v : initializer_list<string_view>{ s... }) {
            stringstream << v << " ";
        }
#ifdef _WIN32
       Window::_log(stringstream.str());
#endif
    }
#endif

    //JPH::Mat44 glmToJolt(const mat4& glmMat);

    /**
     * Split a string
     */
    vector<string_view> split(string_view str, char delimiter);

    /**
     * Returns a random value in the range [0, max]
     */
    uint32_t randomi(uint32_t max = 100);

    /**
     * Returns a random value in the range [0.0f, max]
     */
    float randomf(float max = 100.0f);

}

namespace std {
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

    vec3 to_vec3(const string& str) ;

    /** 
     * lerp for a vec2 using std::lerp for componants
    */
    inline vec2 lerp(vec2 a, vec2 b, float t) {
        return vec2{
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t),
        };
    }

    /** 
     * lerp for a vec3 using std::lerp for componants
    */
    inline vec3 lerp(vec3 a, vec3 b, float t) {
        return vec3{
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t),
            lerp(a.z, b.z, t),
        };
    }

    /** 
     * lerp for a vec4 using std::lerp for componants
    */
    inline vec4 lerp(vec4 a, vec4 b, float t) {
        return vec4{
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t),
            lerp(a.z, b.z, t),
            lerp(a.w, b.w, t),
        };
    }

}