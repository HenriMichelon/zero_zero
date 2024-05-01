#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <volk.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTC_constants
#include <glm/glm.hpp>
using namespace glm;

#include <string>
#include <memory>
using namespace std;

namespace z0 {

    constexpr string ENGINE_NAME = "ZeroZero";
    constexpr int WINDOW_CLEAR_COLOR[] { 0, 0, 0 };

    const vec3 AXIS_X { 1.0, 0.0f, 0.0f };
    const vec3 AXIS_Y { 0.0, 1.0f, 0.0f };
    const vec3 AXIS_Z { 0.0, 0.0f, 1.0f };
    const vec3 AXIS_UP = AXIS_Y;
    const vec3 AXIS_FRONT = -AXIS_Z;
    const vec2 VEC2ZERO{0.0};
    const vec3 VEC3ZERO{0.0};

    enum ProcessMode {
        PROCESS_MODE_INHERIT    = 0,
        PROCESS_MODE_PAUSABLE   = 1,
        PROCESS_MODE_WHEN_PAUSED= 2,
        PROCESS_MODE_ALWAYS     = 3,
        PROCESS_MODE_DISABLED   = 4,
    };

    enum WindowMode {
        WINDOW_MODE_WINDOWED            = 0,
        WINDOW_MODE_WINDOWED_MAXIMIZED  = 1,
        WINDOW_MODE_WINDOWED_FULLSCREEN = 2,
        WINDOW_MODE_FULLSCREEN          = 3,
    };

    enum MSAA {
        MSAA_AUTO       = 0,
        MSAA_2X         = VK_SAMPLE_COUNT_2_BIT,
        MSAA_4X         = VK_SAMPLE_COUNT_4_BIT,
        MSAA_8X         = VK_SAMPLE_COUNT_8_BIT,
    };

    enum CullMode {
        CULLMODE_DISABLED   = 0,
        CULLMODE_BACK       = 1,
        CULLMODE_FRONT      = 2,
    };

    enum Transparency {
        TRANSPARENCY_DISABLED         = 0,
        TRANSPARENCY_ALPHA            = 1, // alpha only
        TRANSPARENCY_SCISSOR          = 2, // scissor onmy
        TRANSPARENCY_SCISSOR_ALPHA    = 3, // scissor then alpha
    };

}