#pragma once

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

    /*enum MSAA {
        MSAA_AUTO       = 0,
        MSAA_2X         = 1,
        MSAA_4X         = 2,
        MSAA_8X         = 3,
    };*/

}