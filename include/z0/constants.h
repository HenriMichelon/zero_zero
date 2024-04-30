#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <volk.h>

#include <string>
#include <memory>
using namespace std;

namespace z0 {

    constexpr string ENGINE_NAME = "ZeroZero";
    constexpr int WINDOW_CLEAR_COLOR[] { 0, 0, 0 };

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

}