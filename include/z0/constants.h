#pragma once

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
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
        TRANSPARENCY_SCISSOR          = 2, // scissor olmy
        TRANSPARENCY_SCISSOR_ALPHA    = 3, // scissor then alpha
    };

    enum InputEventType {
        INPUT_EVENT_KEY             = 0,
        INPUT_EVENT_MOUSE_MOTION    = 1,
        INPUT_EVENT_MOUSE_BUTTON    = 2,
    };

    enum KeyModifier {
        KEY_MODIFIER_SHIFT      = 0x0001,
        KEY_MODIFIER_CONTROL    = 0x0002,
        KEY_MODIFIER_ALT        = 0x0004,
    };

    enum Key {
        KEY_NONE        = 0,
        KEY_SPACE       = 1,
        KEY_DASH        = 2,
        KEY_PIPE        = 3,
        KEY_APOSTROPHE  = 4,
        KEY_COMMA       = 5,
        KEY_PERIOD      = 6,
        KEY_QUESTIONMARK= 7,
        KEY_0           = 8,
        KEY_1           = 9,
        KEY_2           = 10,
        KEY_3           = 11,
        KEY_4           = 12,
        KEY_5           = 13,
        KEY_6           = 14,
        KEY_7           = 15,
        KEY_8           = 16,
        KEY_9           = 17,
        KEY_SEMICOLON   = 18,
        KEY_EQUAL       = 19,
        KEY_A           = 20,
        KEY_B           = 21,
        KEY_C           = 22,
        KEY_D           = 23,
        KEY_E           = 24,
        KEY_F           = 25,
        KEY_G           = 26,
        KEY_H           = 27,
        KEY_I           = 28,
        KEY_J           = 29,
        KEY_K           = 30,
        KEY_L           = 31,
        KEY_M           = 32,
        KEY_N           = 33,
        KEY_O           = 34,
        KEY_P           = 35,
        KEY_Q           = 36,
        KEY_R           = 37,
        KEY_S           = 38,
        KEY_T           = 39,
        KEY_U           = 40,
        KEY_V           = 41,
        KEY_W           = 42,
        KEY_X           = 43,
        KEY_Y           = 44,
        KEY_Z           = 45,
        KEY_LEFT_BRACKET   = 46,
        KEY_BACKSLASH      = 47,
        KEY_RIGHT_BRACKET  = 48,
        KEY_GRAVE_ACCENT   = 49,
        KEY_ESCAPE         = 50,
        KEY_ENTER          = 51,
        KEY_TAB            = 52,
        KEY_BACKSPACE      = 53,
        KEY_INSERT         = 54,
        KEY_DELETE         = 55,
        KEY_RIGHT          = 56,
        KEY_LEFT           = 57,
        KEY_DOWN           = 58,
        KEY_UP             = 59,
        KEY_PAGE_UP        = 60,
        KEY_PAGE_DOWN      = 61,
        KEY_HOME           = 62,
        KEY_END            = 63,
        KEY_CAPS_LOCK      = 64,
        KEY_SCROLL_LOCK    = 65,
        KEY_NUM_LOCK       = 66,
        KEY_PRINT_SCREEN   = 67,
        KEY_PAUSE          = 68,
        KEY_F1             = 69,
        KEY_F2             = 70,
        KEY_F3             = 71,
        KEY_F4             = 72,
        KEY_F5             = 73,
        KEY_F6             = 74,
        KEY_F7             = 75,
        KEY_F8             = 76,
        KEY_F9             = 77,
        KEY_F10            = 78,
        KEY_F11            = 79,
        KEY_F12            = 80,
        KEY_KP_0           = 81,
        KEY_KP_1           = 82,
        KEY_KP_2           = 83,
        KEY_KP_3           = 84,
        KEY_KP_4           = 85,
        KEY_KP_5           = 86,
        KEY_KP_6           = 87,
        KEY_KP_7           = 88,
        KEY_KP_8           = 89,
        KEY_KP_9           = 90,
        KEY_KP_PERIOD      = 91,
        KEY_KP_DIVIDE      = 92,
        KEY_KP_MULTIPLY    = 93,
        KEY_KP_SUBTRACT    = 94,
        KEY_KP_ADD         = 95,
        KEY_KP_ENTER       = 96,
        KEY_KP_EQUAL       = 97,
        KEY_LEFT_SHIFT     = 98,
        KEY_LEFT_CONTROL   = 99,
        KEY_LEFT_ALT       = 100,
        KEY_LEFT_SUPER     = 101,
        KEY_RIGHT_SHIFT    = 102,
        KEY_RIGHT_CONTROL  = 103,
        KEY_RIGHT_ALT      = 104,
        KEY_RIGHT_SUPER    = 105,
    };

#ifdef _WIN32
    enum OsKey {
        // https://learn.microsoft.com/fr-fr/windows/win32/inputdev/about-keyboard-input#scan-codes
        OS_KEY_SPACE      = 0x0039,
        OS_KEY_DASH       = 0x000C,
        OS_KEY_PIPE       = 0x002B,
        OS_KEY_APOSTROPHE = 0x0028,
        OS_KEY_COMMA      = 0x0033,
        OS_KEY_PERIOD     = 0x0034,
        OS_KEY_QUESTIONMARK= 0x0035,
        OS_KEY_0          = 0x000B,
        OS_KEY_1          = 0x0002,
        OS_KEY_2          = 0x0003,
        OS_KEY_3          = 0x0004,
        OS_KEY_4          = 0x0005,
        OS_KEY_5          = 0x0006,
        OS_KEY_6          = 0x0007,
        OS_KEY_7          = 0x0008,
        OS_KEY_8          = 0x0009,
        OS_KEY_9          = 0x000A,
        OS_KEY_SEMICOLON  = 0x0027,
        OS_KEY_EQUAL      = 0x000D,
        OS_KEY_A          = 0x001E,
        OS_KEY_B          = 0x0030,
        OS_KEY_C          = 0x002E,
        OS_KEY_D          = 0x0020,
        OS_KEY_E          = 0x0012,
        OS_KEY_F          = 0x0021,
        OS_KEY_G          = 0x0022,
        OS_KEY_H          = 0x0023,
        OS_KEY_I          = 0x0017,
        OS_KEY_J          = 0x0024,
        OS_KEY_K          = 0x0025,
        OS_KEY_L          = 0x0026,
        OS_KEY_M          = 0x0032,
        OS_KEY_N          = 0x0031,
        OS_KEY_O          = 0x0018,
        OS_KEY_P          = 0x0019,
        OS_KEY_Q          = 0x0010,
        OS_KEY_R          = 0x0013,
        OS_KEY_S          = 0x001F,
        OS_KEY_T          = 0x0014,
        OS_KEY_U          = 0x0016,
        OS_KEY_V          = 0x002F,
        OS_KEY_W          = 0x0011,
        OS_KEY_X          = 0x002D,
        OS_KEY_Y          = 0x0015,
        OS_KEY_Z          = 0x002C,
        OS_KEY_LEFT_BRACKET   = 0x001A,
        OS_KEY_BACKSLASH      = 0x000E,
        OS_KEY_RIGHT_BRACKET  = 0x001B,
        OS_KEY_GRAVE_ACCENT   = 0x0029,
        OS_KEY_ESCAPE         = 0x0001,
        OS_KEY_ENTER          = 0x001C,
        OS_KEY_TAB            = 0x000F,
        OS_KEY_BACKSPACE      = 0x000E,
        OS_KEY_INSERT         = 0xE052,
        OS_KEY_DELETE         = 0xE053,
        OS_KEY_RIGHT          = 0xE04D,
        OS_KEY_LEFT           = 0xE04B,
        OS_KEY_DOWN           = 0xE050,
        OS_KEY_UP             = 0xE048,
        OS_KEY_PAGE_UP        = 0xE049,
        OS_KEY_PAGE_DOWN      = 0xE051,
        OS_KEY_HOME           = 0xE047,
        OS_KEY_END            = 0xE04F,
        OS_KEY_CAPS_LOCK      = 0x003A,
        OS_KEY_SCROLL_LOCK    = 0x0046,
        OS_KEY_NUM_LOCK       = 0x0045,
        OS_KEY_PRINT_SCREEN   = 0xE037,
        OS_KEY_PAUSE          = 0x0045,
        OS_KEY_F1             = 0x003B,
        OS_KEY_F2             = 0x003C,
        OS_KEY_F3             = 0x003D,
        OS_KEY_F4             = 0x003E,
        OS_KEY_F5             = 0x003F,
        OS_KEY_F6             = 0x0040,
        OS_KEY_F7             = 0x0041,
        OS_KEY_F8             = 0x0042,
        OS_KEY_F9             = 0x0043,
        OS_KEY_F10            = 0x0044,
        OS_KEY_F11            = 0x0057,
        OS_KEY_F12            = 0x0058,
        OS_KEY_KP_0           = 0x0052,
        OS_KEY_KP_1           = 0x004F,
        OS_KEY_KP_2           = 0x0050,
        OS_KEY_KP_3           = 0x0051,
        OS_KEY_KP_4           = 0x004B,
        OS_KEY_KP_5           = 0x004C,
        OS_KEY_KP_6           = 0x004D,
        OS_KEY_KP_7           = 0x0047,
        OS_KEY_KP_8           = 0x0048,
        OS_KEY_KP_9           = 0x0049,
        OS_KEY_KP_PERIOD      = 0x0053,
        OS_KEY_KP_DIVIDE      = 0xE035,
        OS_KEY_KP_MULTIPLY    = 0x0037,
        OS_KEY_KP_SUBTRACT    = 0x004A,
        OS_KEY_KP_ADD         = 0x004E,
        OS_KEY_KP_ENTER       = 0xE01C,
        OS_KEY_KP_EQUAL       = 0x0059,
        OS_KEY_LEFT_SHIFT     = 0x002A,
        OS_KEY_LEFT_CONTROL   = 0x001D,
        OS_KEY_LEFT_ALT       = 0x0038,
        OS_KEY_LEFT_SUPER     = 0xE05B,
        OS_KEY_RIGHT_SHIFT    = 0x0036,
        OS_KEY_RIGHT_CONTROL  = 0xE01D,
        OS_KEY_RIGHT_ALT      = 0xE038,
        OS_KEY_RIGHT_SUPER    = 0xE05C,
    };
#endif

    enum MouseButton {
        MOUSE_BUTTON_NONE      = 0,
        MOUSE_BUTTON_LEFT      = 1,
        MOUSE_BUTTON_RIGHT     = 2,
        MOUSE_BUTTON_MIDDLE    = 3,
    };
/*
    enum GamepadButton {
        GAMEPAD_BUTTON_A            = 0,
        GAMEPAD_BUTTON_B            = 1,
        GAMEPAD_BUTTON_X            = 2,
        GAMEPAD_BUTTON_Y            = 3,
        GAMEPAD_BUTTON_LEFT_BUMPER  = 4,
        GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
        GAMEPAD_BUTTON_BACK         = 6,
        GAMEPAD_BUTTON_START        = 7,
        GAMEPAD_BUTTON_GUIDE        = 8,
        GAMEPAD_BUTTON_LEFT_THUMB   = 9,
        GAMEPAD_BUTTON_RIGHT_THUMB  = 10,
        GAMEPAD_BUTTON_DPAD_UP      = 11,
        GAMEPAD_BUTTON_DPAD_RIGHT   = 12,
        GAMEPAD_BUTTON_DPAD_DOWN    = 13,
        GAMEPAD_BUTTON_DPAD_LEFT    = 14,
        GAMEPAD_BUTTON_CROSS        = GAMEPAD_BUTTON_A,
        GAMEPAD_BUTTON_CIRCLE       = GAMEPAD_BUTTON_B,
        GAMEPAD_BUTTON_SQUARE       = GAMEPAD_BUTTON_X,
        GAMEPAD_BUTTON_TRIANGLE     = GAMEPAD_BUTTON_Y,
    };

    enum GamepadAxisJoystick {
        GAMEPAD_AXIS_LEFT         = 0,
        GAMEPAD_AXIS_RIGHT        = 1,
    };

    enum GamepadAxis {
        GAMEPAD_AXIS_LEFT_X         = 0,
        GAMEPAD_AXIS_LEFT_Y         = 1,
        GAMEPAD_AXIS_RIGHT_X        = 2,
        GAMEPAD_AXIS_RIGHT_Y        = 3,
    };
*/

    enum MouseMode {
        MOUSE_MODE_VISIBLE          = 0,
        MOUSE_MODE_VISIBLE_CAPTURED = 1,
        MOUSE_MODE_HIDDEN           = 2,
        MOUSE_MODE_HIDDEN_CAPTURED  = 3,
    };

}