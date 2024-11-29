/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include <dinput.h>
#endif
#include "z0/libraries.h"
#include "mappings.h"

module z0.Input;

import z0.Constants;
import z0.InputEvent;
import z0.Tools;
import z0.Application;

namespace z0 {

    bool Input::isKeyPressed(const Key key) {
        return _keyPressedStates[key];
    }

    bool Input::isKeyJustPressed(const Key key) {
        const auto result          = _keyJustPressedStates[key];
        _keyJustPressedStates[key] = false;
        return result;
    }

    bool Input::isKeyJustReleased(const Key key) {
        const auto result           = _keyJustReleasedStates[key];
        _keyJustReleasedStates[key] = false;
        return result;
    }

    bool Input::isMouseButtonPressed(const MouseButton mouseButton) {
        return _mouseButtonPressedStates[mouseButton];
    }

    bool Input::isMouseButtonJustPressed(const MouseButton mouseButton) {
        const auto result                          = _mouseButtonJustPressedStates[mouseButton];
        _mouseButtonJustPressedStates[mouseButton] = false;
        return result;
    }

    bool Input::isMouseButtonJustReleased(const MouseButton mouseButton) {
        const auto result                           = _mouseButtonJustReleasedStates[mouseButton];
        _mouseButtonJustReleasedStates[mouseButton] = false;
        return result;
    }

    unordered_map<Key, bool>           Input::_keyPressedStates;
    unordered_map<Key, bool>           Input::_keyJustPressedStates;
    unordered_map<Key, bool>           Input::_keyJustReleasedStates;
    unordered_map<MouseButton, bool>   Input::_mouseButtonPressedStates;
    unordered_map<MouseButton, bool>   Input::_mouseButtonJustPressedStates;
    unordered_map<MouseButton, bool>   Input::_mouseButtonJustReleasedStates;
    unordered_map<GamepadButton, bool> Input::_gamepadButtonPressedStates;

    static map<Key, OsKey> _keyMap{
            {KEY_SPACE, OS_KEY_SPACE},
            {KEY_DASH, OS_KEY_DASH},
            {KEY_PIPE, OS_KEY_PIPE},
            {KEY_APOSTROPHE, OS_KEY_APOSTROPHE},
            {KEY_COMMA, OS_KEY_COMMA},
            {KEY_PERIOD, OS_KEY_PERIOD},
            {KEY_QUESTIONMARK, OS_KEY_QUESTIONMARK},
            {KEY_0, OS_KEY_0},
            {KEY_1, OS_KEY_1},
            {KEY_2, OS_KEY_2},
            {KEY_3, OS_KEY_3},
            {KEY_4, OS_KEY_4},
            {KEY_5, OS_KEY_5},
            {KEY_6, OS_KEY_6},
            {KEY_7, OS_KEY_7},
            {KEY_8, OS_KEY_8},
            {KEY_9, OS_KEY_9},
            {KEY_SEMICOLON, OS_KEY_SEMICOLON},
            {KEY_EQUAL, OS_KEY_EQUAL},
            {KEY_A, OS_KEY_A},
            {KEY_B, OS_KEY_B},
            {KEY_C, OS_KEY_C},
            {KEY_D, OS_KEY_D},
            {KEY_E, OS_KEY_E},
            {KEY_F, OS_KEY_F},
            {KEY_G, OS_KEY_G},
            {KEY_H, OS_KEY_H},
            {KEY_I, OS_KEY_I},
            {KEY_J, OS_KEY_J},
            {KEY_K, OS_KEY_K},
            {KEY_L, OS_KEY_L},
            {KEY_M, OS_KEY_M},
            {KEY_N, OS_KEY_N},
            {KEY_O, OS_KEY_O},
            {KEY_P, OS_KEY_P},
            {KEY_Q, OS_KEY_Q},
            {KEY_R, OS_KEY_R},
            {KEY_S, OS_KEY_S},
            {KEY_T, OS_KEY_T},
            {KEY_U, OS_KEY_U},
            {KEY_V, OS_KEY_V},
            {KEY_W, OS_KEY_W},
            {KEY_X, OS_KEY_X},
            {KEY_Y, OS_KEY_Y},
            {KEY_Z, OS_KEY_Z},
            {KEY_LEFT_BRACKET, OS_KEY_LEFT_BRACKET},
            //{ KEY_BACKSLASH      , OS_KEY_BACKSLASH },
            {KEY_RIGHT_BRACKET, OS_KEY_RIGHT_BRACKET},
            {KEY_GRAVE_ACCENT, OS_KEY_GRAVE_ACCENT},
            {KEY_ESCAPE, OS_KEY_ESCAPE},
            {KEY_ENTER, OS_KEY_ENTER},
            {KEY_TAB, OS_KEY_TAB},
            {KEY_BACKSPACE, OS_KEY_BACKSPACE},
            {KEY_INSERT, OS_KEY_INSERT},
            {KEY_DELETE, OS_KEY_DELETE},
            {KEY_RIGHT, OS_KEY_RIGHT},
            {KEY_LEFT, OS_KEY_LEFT},
            {KEY_DOWN, OS_KEY_DOWN},
            {KEY_UP, OS_KEY_UP},
            {KEY_PAGE_UP, OS_KEY_PAGE_UP},
            {KEY_PAGE_DOWN, OS_KEY_PAGE_DOWN},
            {KEY_HOME, OS_KEY_HOME},
            {KEY_END, OS_KEY_END},
            {KEY_CAPS_LOCK, OS_KEY_CAPS_LOCK},
            {KEY_SCROLL_LOCK, OS_KEY_SCROLL_LOCK},
            {KEY_NUM_LOCK, OS_KEY_NUM_LOCK},
            {KEY_PRINT_SCREEN, OS_KEY_PRINT_SCREEN},
            {KEY_PAUSE, OS_KEY_PAUSE},
            {KEY_F1, OS_KEY_F1},
            {KEY_F2, OS_KEY_F2},
            {KEY_F3, OS_KEY_F3},
            {KEY_F4, OS_KEY_F4},
            {KEY_F5, OS_KEY_F5},
            {KEY_F6, OS_KEY_F6},
            {KEY_F7, OS_KEY_F7},
            {KEY_F8, OS_KEY_F8},
            {KEY_F9, OS_KEY_F9},
            {KEY_F10, OS_KEY_F10},
            {KEY_F11, OS_KEY_F11},
            {KEY_F12, OS_KEY_F12},
            {KEY_KP_0, OS_KEY_KP_0},
            {KEY_KP_1, OS_KEY_KP_1},
            {KEY_KP_2, OS_KEY_KP_2},
            {KEY_KP_3, OS_KEY_KP_3},
            {KEY_KP_4, OS_KEY_KP_4},
            {KEY_KP_5, OS_KEY_KP_5},
            {KEY_KP_6, OS_KEY_KP_6},
            {KEY_KP_7, OS_KEY_KP_7},
            {KEY_KP_8, OS_KEY_KP_8},
            {KEY_KP_9, OS_KEY_KP_9},
            {KEY_KP_PERIOD, OS_KEY_KP_PERIOD},
            {KEY_KP_DIVIDE, OS_KEY_KP_DIVIDE},
            {KEY_KP_MULTIPLY, OS_KEY_KP_MULTIPLY},
            {KEY_KP_SUBTRACT, OS_KEY_KP_SUBTRACT},
            {KEY_KP_ADD, OS_KEY_KP_ADD},
            {KEY_KP_ENTER, OS_KEY_KP_ENTER},
            {KEY_KP_EQUAL, OS_KEY_KP_EQUAL},
            {KEY_LEFT_SHIFT, OS_KEY_LEFT_SHIFT},
            {KEY_LEFT_CONTROL, OS_KEY_LEFT_CONTROL},
            {KEY_LEFT_ALT, OS_KEY_LEFT_ALT},
            {KEY_LEFT_SUPER, OS_KEY_LEFT_SUPER},
            {KEY_RIGHT_SHIFT, OS_KEY_RIGHT_SHIFT},
            {KEY_RIGHT_CONTROL, OS_KEY_RIGHT_CONTROL},
            {KEY_RIGHT_ALT, OS_KEY_RIGHT_ALT},
            {KEY_RIGHT_SUPER, OS_KEY_RIGHT_SUPER},
    };

    OsKey Input::keyToOsKey(Key key) {
        return _keyMap[key];
    }

    Key Input::osKeyToKey(OsKey key) {
        auto it = find_if(_keyMap.begin(),
                          _keyMap.end(),
                          [&key] (const auto &p) {
                              return p.second == key;
                          });

        if (it != _keyMap.end())
            return it->first;
        return KEY_NONE;
    }

    float Input::applyDeadzone(const float value, const float deadzonePercent) {
        if (abs(value) < deadzonePercent) {
            return 0.0f;
        }
        return std::min(std::max(value < 0.0f ? value + deadzonePercent : value - deadzonePercent, -1.0f), 1.0f);
    }

#ifdef _WIN32

    bool        Input::_useXInput{false};
    const int   Input::DI_AXIS_RANGE     = 1000;
    const float Input::DI_AXIS_RANGE_DIV = 1000.5f;

    map<MouseCursor, HCURSOR> Input::_mouseCursors;

    struct _DirectInputState {
        LPDIRECTINPUTDEVICE8 device;
        string name;
        array<float, 6> axes; // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
        array<bool, 32> buttons;
        // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
        int                                 indexAxisLeftX{0};
        int                                 indexAxisLeftY{0};
        int                                 indexAxisRightX{0};
        int                                 indexAxisRightY{0};
        array<int, static_cast<int>(GamepadButton::LAST) + 1> indexButtons;
    };

    static map<uint32_t, _DirectInputState> _directInputStates{};
    static map<uint32_t, XINPUT_STATE>      _xinputStates{};
    static LPDIRECTINPUT8                   _directInput = nullptr;

    static map<GamepadButton, int> GAMEPABUTTON2XINPUT{
            {GamepadButton::A, XINPUT_GAMEPAD_A},
            {GamepadButton::B, XINPUT_GAMEPAD_B},
            {GamepadButton::X, XINPUT_GAMEPAD_X},
            {GamepadButton::Y, XINPUT_GAMEPAD_Y},
            {GamepadButton::LB, XINPUT_GAMEPAD_LEFT_SHOULDER},
            {GamepadButton::RB, XINPUT_GAMEPAD_RIGHT_SHOULDER},
            {GamepadButton::LT, XINPUT_GAMEPAD_LEFT_THUMB},
            {GamepadButton::RT, XINPUT_GAMEPAD_RIGHT_THUMB},
            {GamepadButton::BACK, XINPUT_GAMEPAD_BACK},
            {GamepadButton::START, XINPUT_GAMEPAD_START},
            {GamepadButton::DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN},
            {GamepadButton::DPAD_UP, XINPUT_GAMEPAD_DPAD_UP},
            {GamepadButton::DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT},
            {GamepadButton::DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT},
            {GamepadButton::DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT},
    };

    static BOOL CALLBACK _deviceObjectCallback(const DIDEVICEOBJECTINSTANCEA *doi,
                                               void *                         user) {
        auto *data = reinterpret_cast<_DirectInputState *>(user);
        if (DIDFT_GETTYPE(doi->dwType) & DIDFT_AXIS) {
            DIPROPRANGE dipr;
            ZeroMemory(&dipr, sizeof(dipr));
            dipr.diph.dwSize       = sizeof(dipr);
            dipr.diph.dwHeaderSize = sizeof(dipr.diph);
            dipr.diph.dwObj        = doi->dwType;
            dipr.diph.dwHow        = DIPH_BYID;
            dipr.lMin              = -Input::DI_AXIS_RANGE;
            dipr.lMax              = Input::DI_AXIS_RANGE;
            if (FAILED(data->device->SetProperty(DIPROP_RANGE, &dipr.diph))) {
                return DIENUM_CONTINUE;
            }
        }
        return DIENUM_CONTINUE;
    }

    BOOL CALLBACK _enumGamepadsCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext) {
        if (_directInput) {
            _DirectInputState state{
                    .device = nullptr,
                    .name = string{pdidInstance->tszProductName}
            };
            if (FAILED(_directInput->CreateDevice(pdidInstance->guidInstance,
                &state.device,
                nullptr))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->SetDataFormat(&c_dfDIJoystick))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->SetCooperativeLevel(nullptr, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                return DIENUM_CONTINUE;
            }
            if (FAILED(state.device->Acquire())) {
                return DIENUM_CONTINUE;
            }

            if (FAILED(state.device->EnumObjects(_deviceObjectCallback,
                &state,
                DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV))) {
                state.device->Release();
                return DIENUM_CONTINUE;
            }

            // Generate a joystick GUID that matches the SDL 2.0.5+ one
            // https://github.com/glfw/glfw/blob/master/src/win32_joystick.c#L452
            char guid[33];
            char name[256];
            if (!WideCharToMultiByte(CP_UTF8,
                                     0,
                                     reinterpret_cast<LPCWCH>(pdidInstance->tszInstanceName),
                                     -1,
                                     name,
                                     sizeof(name),
                                     nullptr,
                                     nullptr)) {
                state.device->Release();
                return DIENUM_CONTINUE;
            }
            if (memcmp(&pdidInstance->guidProduct.Data4[2], "PIDVID", 6) == 0) {
                sprintf(guid,
                        "03000000%02x%02x0000%02x%02x000000000000",
                        static_cast<uint8_t>(pdidInstance->guidProduct.Data1),
                        static_cast<uint8_t>(pdidInstance->guidProduct.Data1 >> 8),
                        static_cast<uint8_t>(pdidInstance->guidProduct.Data1 >> 16),
                        static_cast<uint8_t>(pdidInstance->guidProduct.Data1 >> 24));
            } else {
                sprintf(guid,
                        "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                        name[0],
                        name[1],
                        name[2],
                        name[3],
                        name[4],
                        name[5],
                        name[6],
                        name[7],
                        name[8],
                        name[9],
                        name[10]);
            }

            auto sguid = string{guid};
            for (auto &_inputDefaultMapping : _inputDefaultMappings) {
                auto mapping = split(_inputDefaultMapping, ',');
                if (mapping[0] == sguid) {
                    for (int i = 2; i < mapping.size(); i++) {
                        auto parts = split(mapping[i], ':');
                        if ((parts.size() > 1) && (parts[1].size() > 1)) {
                            try {
                                auto index = stoi(string{parts[1].substr(1)});
                                log(parts[0], "/", parts[1]);
                                if (parts[0] == "leftx")
                                    state.indexAxisLeftX = index;
                                if (parts[0] == "lefty")
                                    state.indexAxisLeftY = index;
                                if (parts[0] == "rightx")
                                    state.indexAxisRightX = index;
                                if (parts[0] == "righty")
                                    state.indexAxisRightY = index;
                                if (parts[0] == "a")
                                    state.indexButtons[static_cast<int>(GamepadButton::A)] = index;
                                if (parts[0] == "b")
                                    state.indexButtons[static_cast<int>(GamepadButton::B)] = index;
                                if (parts[0] == "x")
                                    state.indexButtons[static_cast<int>(GamepadButton::X)] = index;
                                if (parts[0] == "y")
                                    state.indexButtons[static_cast<int>(GamepadButton::Y)] = index;
                                if (parts[0] == "leftshoulder")
                                    state.indexButtons[static_cast<int>(GamepadButton::LB)] = index;
                                if (parts[0] == "rightshoulder")
                                    state.indexButtons[static_cast<int>(GamepadButton::RB)] = index;
                                if (parts[0] == "leftstick")
                                    state.indexButtons[static_cast<int>(GamepadButton::LT)] = index;
                                if (parts[0] == "rightstick")
                                    state.indexButtons[static_cast<int>(GamepadButton::RT)] = index;
                                if (parts[0] == "back")
                                    state.indexButtons[static_cast<int>(GamepadButton::BACK)] = index;
                                if (parts[0] == "start")
                                    state.indexButtons[static_cast<int>(GamepadButton::START)] = index;
                            } catch (const std::invalid_argument &e) {
                            }
                        }
                    }
                    _directInputStates[_directInputStates.size()] = state;
                }
            }
        }
        return DIENUM_CONTINUE;
    }

    void Input::_initInput() {
        _mouseCursors[MouseCursor::ARROW]    = LoadCursor(nullptr, IDC_ARROW);
        _mouseCursors[MouseCursor::WAIT]     = LoadCursor(nullptr, IDC_WAIT);
        _mouseCursors[MouseCursor::RESIZE_H] = LoadCursor(nullptr, IDC_SIZEWE);
        _mouseCursors[MouseCursor::RESIZE_V] = LoadCursor(nullptr, IDC_SIZENS);

        for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                _xinputStates[i] = state;
            }
        }
        _useXInput = !_xinputStates.empty();
        if (!_useXInput) {
            if (FAILED(DirectInput8Create(GetModuleHandle(nullptr),
                DIRECTINPUT_VERSION,
                IID_IDirectInput8,
                reinterpret_cast<void**>(&_directInput), nullptr))) {
                die("DirectInput8Create failed");
            }
            _directInput->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                      _enumGamepadsCallback,
                                      nullptr,
                                      DIEDFL_ATTACHEDONLY);
        }
    }

    void Input::_closeInput() {
        if (_directInput) {
            for (const auto entry : _directInputStates) {
                entry.second.device->Release();
            }
            _directInputStates.clear();
            _directInput->Release();
            _directInput = nullptr;
        }
    }

    uint32_t Input::getConnectedJoypads() {
        uint32_t count = 0;
        if (_useXInput) {
            count = _xinputStates.size();
        } else {
            count = _directInputStates.size();
        }
        return count;
    }

    bool Input::isGamepad(const uint32_t index) {
        if (_useXInput) {
            if (_xinputStates.contains(index)) {
                XINPUT_CAPABILITIES xinputCapabilities;
                ZeroMemory(&xinputCapabilities, sizeof(XINPUT_CAPABILITIES));
                if (XInputGetCapabilities(index, 0, &xinputCapabilities) == ERROR_SUCCESS) {
                    return xinputCapabilities.SubType == XINPUT_DEVSUBTYPE_GAMEPAD;
                }
            }
            return false;
        } else {
            return _directInputStates.contains(index);
        }
    }

    void Input::generateGamepadButtonEvent(const GamepadButton button, const bool pressed) {
        if (pressed && (!_gamepadButtonPressedStates[button])) {
            auto event = InputEventGamepadButton(button, pressed);
            Application::get()._onInput(event);
        }
        if ((!pressed) && (_gamepadButtonPressedStates[button])) {
            auto event = InputEventGamepadButton(button, pressed);
            Application::get()._onInput(event);
        }
        _gamepadButtonPressedStates[button] = pressed;
    }

    void Input::_updateInputStates() {
        if (_useXInput) {
            _xinputStates.clear();
            for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
                XINPUT_STATE state;
                ZeroMemory(&state, sizeof(XINPUT_STATE));
                if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                    _xinputStates[i] = state;
                    for (int i = 0; i < static_cast<int>(GamepadButton::LAST); i++) {
                        auto button = static_cast<GamepadButton>(i);
                        generateGamepadButtonEvent(button, state.Gamepad.wButtons & GAMEPABUTTON2XINPUT[button]);
                    }
                }
            }
        } else {
            for (auto &entry : _directInputStates) {
                auto &  gamepad = entry.second;
                HRESULT hr      = gamepad.device->Poll();
                if (FAILED(hr)) {
                    hr = gamepad.device->Acquire();
                    while (hr == DIERR_INPUTLOST) {
                        hr = gamepad.device->Acquire();
                    }
                    if (FAILED(hr)) {
                        _directInputStates.erase(entry.first);
                        continue;
                    }
                }

                DIJOYSTATE state{0};
                if (FAILED(gamepad.device->GetDeviceState(sizeof(state), &state))) {
                    continue;
                }
                gamepad.axes[0] = (static_cast<float>(state.lX) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[1] = (static_cast<float>(state.lY) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[2] = (static_cast<float>(state.lZ) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[3] = (static_cast<float>(state.lRx) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[4] = (static_cast<float>(state.lRy) + 0.5f) / DI_AXIS_RANGE_DIV;
                gamepad.axes[5] = (static_cast<float>(state.lRz) + 0.5f) / DI_AXIS_RANGE_DIV;
                for (int i = 0; i < 32; i++) {
                    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416627(v=vs.85)
                    gamepad.buttons[i] = (state.rgbButtons[i] & 0x80);
                }
                for (int i = 0; i < static_cast<int>(GamepadButton::LAST); i++) {
                    auto button = static_cast<GamepadButton>(i);
                    generateGamepadButtonEvent(button, gamepad.buttons[gamepad.indexButtons[static_cast<int>(button)]]);
                }
            }
        }
    }

    bool Input::isGamepadButtonPressed(const uint32_t index, const GamepadButton gamepadButton) {
        if (_useXInput && _xinputStates.contains(index)) {
            return _xinputStates[index].Gamepad.wButtons & GAMEPABUTTON2XINPUT[gamepadButton];
        } else if (_directInputStates.contains(index)) {
            const auto &gamepad = _directInputStates[index];
            return gamepad.buttons[gamepad.indexButtons[static_cast<int>(gamepadButton)]];
        }
        return false;
    }

    vec2 Input::getGamepadVector(const uint32_t index, const GamepadAxisJoystick axisJoystick) {
        if (_useXInput && _xinputStates.contains(index)) {
            const auto gamepad        = _xinputStates[index].Gamepad;
            const auto xAxis          = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.sThumbLX : gamepad.sThumbRX;
            const auto yAxis          = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.sThumbLY : gamepad.sThumbRY;
            const auto deadzonPercent = ((axisJoystick == GamepadAxisJoystick::LEFT
                    ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
                    : XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
                / 32767.0f);
            vec2 vector{
                    applyDeadzone(xAxis / 32767.0f, deadzonPercent),
                    applyDeadzone(-yAxis / 32767.0f, deadzonPercent)
            };
            const float length = glm::length(vector);
            return (length > 1.0f) ? vector / length : vector;
        } else if (_directInputStates.contains(index)) {
            const auto &gamepad = _directInputStates[index];
            const auto  xAxis   = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.indexAxisLeftX : gamepad.indexAxisRightX;
            const auto  yAxis   = axisJoystick == GamepadAxisJoystick::LEFT ? gamepad.indexAxisLeftY : gamepad.indexAxisRightY;
            const vec2  vector{
                    applyDeadzone(gamepad.axes[xAxis], 0.05f),
                    applyDeadzone(gamepad.axes[yAxis], 0.05f)
            };
            const float length = glm::length(vector);
            return (length > 1.0f) ? vector / length : vector;
        }
        return VEC2ZERO;
    }

    string Input::getJoypadName(const uint32_t index) {
        if (_useXInput) {
            return "XInput";
        }
        if (_directInputStates.contains(index)) {
            return _directInputStates[index].name;
        }
        return "??";
    }

    vec2 Input::getKeyboardVector(const Key keyNegX, const Key keyPosX, const Key keyNegY, const Key keyPosY) {
        const auto  x = _keyPressedStates[keyNegX] ? -1 : _keyPressedStates[keyPosX] ? 1 : 0;
        const auto  y = _keyPressedStates[keyNegY] ? -1 : _keyPressedStates[keyPosY] ? 1 : 0;
        const vec2  vector{x, y};
        const float length = glm::length(vector);
        return (length > 1.0f) ? vector / length : vector;
    }

    void Input::setMouseCursor(const MouseCursor cursor) {
        SetCursor(_mouseCursors[cursor]);
        auto &wnd = Application::get().getWindow();
        PostMessage(wnd._getHandle(), WM_SETCURSOR, 0, 0);
    }

    void Input::setMouseMode(const MouseMode mode) {
        auto &wnd = Application::get().getWindow();
        MSG   msg;
        while (PeekMessageW(&msg, wnd._getHandle(), 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        switch (mode) {
        case MouseMode::VISIBLE:
            ReleaseCapture();
            ClipCursor(nullptr);
            ShowCursor(TRUE);
            SetCursorPos(wnd._getRect().left + wnd.getWidth() / 2,
                         wnd._getRect().top + wnd.getHeight() / 2);
            break;
        case MouseMode::HIDDEN:
            ReleaseCapture();
            ClipCursor(nullptr);
            ShowCursor(FALSE);
            break;
        case MouseMode::VISIBLE_CAPTURED: {
            auto rect = wnd._getRect();
            SetCapture(wnd._getHandle());
            ClipCursor(&rect);
            ShowCursor(TRUE);
            SetCursorPos(wnd._getRect().left + wnd.getWidth() / 2,
                         wnd._getRect().top + wnd.getHeight() / 2);
            break;
        }
        case MouseMode::HIDDEN_CAPTURED: {
            auto rect = wnd._getRect();
            SetCapture(wnd._getHandle());
            ClipCursor(&rect);
            ShowCursor(FALSE);
            break;
        }
        default:
            die("Unknown mouse mode");
        }
    }
#endif
}
