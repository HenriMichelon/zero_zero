#pragma once

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <windows.h>
#endif
#include <volk.h>
#include <cassert>
