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
        TRANSPARENCY_SCISSOR          = 2, // scissor onmy
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

/*
    enum Key {
        KEY_SPACE      = 1,
        KEY_DASH       = 2,
        KEY_PIPE       = 3,
        KEY_APOSTROPHE = 4,
        KEY_COMMA      = 5,
        KEY_PERIOD     = 6,
        KEY_QUESTIONMARK= 7,
        KEY_0          = 8,
        KEY_1          = 9,
        KEY_2          = 10,
        KEY_3          = 11,
        KEY_4          = 12,
        KEY_5          = 13,
        KEY_6          = 14,
        KEY_7          = 15,
        KEY_8          = 16,
        KEY_9          = 17,
        KEY_SEMICOLON  = 18,
        KEY_EQUAL      = 19,
        KEY_A          = 0x001E,
        KEY_B          = 0x0030,
        KEY_C          = 0x002E,
        KEY_D          = 0x0020,
        KEY_E          = 0x0012,
        KEY_F          = 0x0021,
        KEY_G          = 0x0022,
        KEY_H          = 0x0023,
        KEY_I          = 0x0017,
        KEY_J          = 0x0024,
        KEY_K          = 0x0025,
        KEY_L          = 0x0026,
        KEY_M          = 0x0032,
        KEY_N          = 0x0031,
        KEY_O          = 0x0018,
        KEY_P          = 0x0019,
        KEY_Q          = 0x0010,
        KEY_R          = 0x0013,
        KEY_S          = 0x001F,
        KEY_T          = 0x0014,
        KEY_U          = 0x0016,
        KEY_V          = 0x002F,
        KEY_W          = 0x0011,
        KEY_X          = 0x002D,
        KEY_Y          = 0x0015,
        KEY_Z          = 0x002C,
        KEY_LEFT_BRACKET   = 0x001A,
        KEY_BACKSLASH      = 0x000E,
        KEY_RIGHT_BRACKET  = 0x001B,
        KEY_GRAVE_ACCENT   = 0x0029,
        KEY_ESCAPE         = 0x0001,
        KEY_ENTER          = 0x001C,
        KEY_TAB            = 0x000F,
        KEY_BACKSPACE      = 0x000E,
        KEY_INSERT         = 0xE052,
        KEY_DELETE         = 0xE053,
        KEY_RIGHT          = 0xE04D,
        KEY_LEFT           = 0xE04B,
        KEY_DOWN           = 0xE050,
        KEY_UP             = 0xE048,
        KEY_PAGE_UP        = 0xE049,
        KEY_PAGE_DOWN      = 0xE051,
        KEY_HOME           = 0xE047,
        KEY_END            = 0xE04F,
        KEY_CAPS_LOCK      = 0x003A,
        KEY_SCROLL_LOCK    = 0x0046,
        KEY_NUM_LOCK       = 0x0045,
        KEY_PRINT_SCREEN   = 0xE037,
        KEY_PAUSE          = 0x0045,
        KEY_F1             = 0x003B,
        KEY_F2             = 0x003C,
        KEY_F3             = 0x003D,
        KEY_F4             = 0x003E,
        KEY_F5             = 0x003F,
        KEY_F6             = 0x0040,
        KEY_F7             = 0x0041,
        KEY_F8             = 0x0042,
        KEY_F9             = 0x0043,
        KEY_F10            = 0x0044,
        KEY_F11            = 0x0057,
        KEY_F12            = 0x0058,
        KEY_KP_0           = 0x0052,
        KEY_KP_1           = 0x004F,
        KEY_KP_2           = 0x0050,
        KEY_KP_3           = 0x0051,
        KEY_KP_4           = 0x004B,
        KEY_KP_5           = 0x004C,
        KEY_KP_6           = 0x004D,
        KEY_KP_7           = 0x0047,
        KEY_KP_8           = 0x0048,
        KEY_KP_9           = 0x0049,
        KEY_KP_PERIOD      = 0x0053,
        KEY_KP_DIVIDE      = 0xE035,
        KEY_KP_MULTIPLY    = 0x0037,
        KEY_KP_SUBTRACT    = 0x004A,
        KEY_KP_ADD         = 0x004E,
        KEY_KP_ENTER       = 0xE01C,
        KEY_KP_EQUAL       = 0x0059,
        KEY_LEFT_SHIFT     = 0x002A,
        KEY_LEFT_CONTROL   = 0x001D,
        KEY_LEFT_ALT       = 0x0038,
        KEY_LEFT_SUPER     = 0xE05B,
        KEY_RIGHT_SHIFT    = 0x0036,
        KEY_RIGHT_CONTROL  = 0xE01D,
        KEY_RIGHT_ALT      = 0xE038,
        KEY_RIGHT_SUPER    = 0xE05C,
    };
    */
#ifdef _WIN32
    enum Key {
        // https://learn.microsoft.com/fr-fr/windows/win32/inputdev/about-keyboard-input#scan-codes
        KEY_SPACE      = 0x0039,
        KEY_DASH       = 0x000C,
        KEY_PIPE       = 0x002B,
        KEY_APOSTROPHE = 0x0028,
        KEY_COMMA      = 0x0033,
        KEY_PERIOD     = 0x0034,
        KEY_QUESTIONMARK= 0x0035,
        KEY_0          = 0x000B,
        KEY_1          = 0x0002,
        KEY_2          = 0x0003,
        KEY_3          = 0x0004,
        KEY_4          = 0x0005,
        KEY_5          = 0x0006,
        KEY_6          = 0x0007,
        KEY_7          = 0x0008,
        KEY_8          = 0x0009,
        KEY_9          = 0x000A,
        KEY_SEMICOLON  = 0x0027,
        KEY_EQUAL      = 0x000D,
        KEY_A          = 0x001E,
        KEY_B          = 0x0030,
        KEY_C          = 0x002E,
        KEY_D          = 0x0020,
        KEY_E          = 0x0012,
        KEY_F          = 0x0021,
        KEY_G          = 0x0022,
        KEY_H          = 0x0023,
        KEY_I          = 0x0017,
        KEY_J          = 0x0024,
        KEY_K          = 0x0025,
        KEY_L          = 0x0026,
        KEY_M          = 0x0032,
        KEY_N          = 0x0031,
        KEY_O          = 0x0018,
        KEY_P          = 0x0019,
        KEY_Q          = 0x0010,
        KEY_R          = 0x0013,
        KEY_S          = 0x001F,
        KEY_T          = 0x0014,
        KEY_U          = 0x0016,
        KEY_V          = 0x002F,
        KEY_W          = 0x0011,
        KEY_X          = 0x002D,
        KEY_Y          = 0x0015,
        KEY_Z          = 0x002C,
        KEY_LEFT_BRACKET   = 0x001A,
        KEY_BACKSLASH      = 0x000E,
        KEY_RIGHT_BRACKET  = 0x001B,
        KEY_GRAVE_ACCENT   = 0x0029,
        KEY_ESCAPE         = 0x0001,
        KEY_ENTER          = 0x001C,
        KEY_TAB            = 0x000F,
        KEY_BACKSPACE      = 0x000E,
        KEY_INSERT         = 0xE052,
        KEY_DELETE         = 0xE053,
        KEY_RIGHT          = 0xE04D,
        KEY_LEFT           = 0xE04B,
        KEY_DOWN           = 0xE050,
        KEY_UP             = 0xE048,
        KEY_PAGE_UP        = 0xE049,
        KEY_PAGE_DOWN      = 0xE051,
        KEY_HOME           = 0xE047,
        KEY_END            = 0xE04F,
        KEY_CAPS_LOCK      = 0x003A,
        KEY_SCROLL_LOCK    = 0x0046,
        KEY_NUM_LOCK       = 0x0045,
        KEY_PRINT_SCREEN   = 0xE037,
        KEY_PAUSE          = 0x0045,
        KEY_F1             = 0x003B,
        KEY_F2             = 0x003C,
        KEY_F3             = 0x003D,
        KEY_F4             = 0x003E,
        KEY_F5             = 0x003F,
        KEY_F6             = 0x0040,
        KEY_F7             = 0x0041,
        KEY_F8             = 0x0042,
        KEY_F9             = 0x0043,
        KEY_F10            = 0x0044,
        KEY_F11            = 0x0057,
        KEY_F12            = 0x0058,
        KEY_KP_0           = 0x0052,
        KEY_KP_1           = 0x004F,
        KEY_KP_2           = 0x0050,
        KEY_KP_3           = 0x0051,
        KEY_KP_4           = 0x004B,
        KEY_KP_5           = 0x004C,
        KEY_KP_6           = 0x004D,
        KEY_KP_7           = 0x0047,
        KEY_KP_8           = 0x0048,
        KEY_KP_9           = 0x0049,
        KEY_KP_PERIOD      = 0x0053,
        KEY_KP_DIVIDE      = 0xE035,
        KEY_KP_MULTIPLY    = 0x0037,
        KEY_KP_SUBTRACT    = 0x004A,
        KEY_KP_ADD         = 0x004E,
        KEY_KP_ENTER       = 0xE01C,
        KEY_KP_EQUAL       = 0x0059,
        KEY_LEFT_SHIFT     = 0x002A,
        KEY_LEFT_CONTROL   = 0x001D,
        KEY_LEFT_ALT       = 0x0038,
        KEY_LEFT_SUPER     = 0xE05B,
        KEY_RIGHT_SHIFT    = 0x0036,
        KEY_RIGHT_CONTROL  = 0xE01D,
        KEY_RIGHT_ALT      = 0xE038,
        KEY_RIGHT_SUPER    = 0xE05C,
    };
#endif


    enum MouseButton {
        MOUSE_BUTTON_LEFT      = 0,
        MOUSE_BUTTON_RIGHT     = 1,
        MOUSE_BUTTON_MIDDLE    = 2,
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