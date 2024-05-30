#include "z0/application.h"
#include "z0/input.h"

#include <unordered_map>

namespace z0 {

    list<shared_ptr<InputEvent>> Input::_inputQueue;
    unordered_map<Key, bool> Input::_keyPressedStates;
    unordered_map<Key, bool> Input::_keyJustPressedStates;
    unordered_map<Key, bool> Input::_keyJustReleasedStates;
    unordered_map<MouseButton, bool> Input::_mouseButtonPressedStates;
    unordered_map<MouseButton, bool> Input::_mouseButtonJustPressedStates;
    unordered_map<MouseButton, bool> Input::_mouseButtonJustReleasedStates;

    void Input::injectInputEvent(const shared_ptr<InputEvent>& event) {
        _inputQueue.push_back(event);
    }

    shared_ptr<InputEvent> Input::consumeInputEvent() {
        const auto& event = _inputQueue.front();
        _inputQueue.pop_front();
        return event;
    }

    bool Input::isMouseButtonPressed(MouseButton mouseButton) {
        return _mouseButtonPressedStates[mouseButton];
    }

    bool Input::isMouseButtonJustPressed(MouseButton mouseButton) {
        auto result = _mouseButtonJustPressedStates[mouseButton];
        _mouseButtonJustPressedStates[mouseButton] = false;
        return result;
    }

    bool Input::isMouseButtonJustReleased(MouseButton mouseButton) {
        auto result = _mouseButtonJustReleasedStates[mouseButton];
        _mouseButtonJustReleasedStates[mouseButton] = false;
        return result;
    }

    bool Input::isKeyPressed(Key key) {
        return _keyPressedStates[key];
    }

    bool Input::isKeyJustPressed(Key key) {
        auto result = _keyJustPressedStates[key];
        _keyJustPressedStates[key] = false;
        return result;
    }

    bool Input::isKeyJustReleased(Key key) {
        auto result = _keyJustReleasedStates[key];
        _keyJustReleasedStates[key] = false;
        return result;
    }

    static map<Key, OsKey> _keyMap {
            { KEY_SPACE       , OS_KEY_SPACE },
            { KEY_DASH        , OS_KEY_DASH },
            { KEY_PIPE        , OS_KEY_PIPE },
            { KEY_APOSTROPHE  , OS_KEY_APOSTROPHE },
            { KEY_COMMA       , OS_KEY_COMMA },
            { KEY_PERIOD      , OS_KEY_PERIOD },
            { KEY_QUESTIONMARK, OS_KEY_QUESTIONMARK },
            { KEY_0           , OS_KEY_0 },
            { KEY_1           , OS_KEY_1 },
            { KEY_2           , OS_KEY_2 },
            { KEY_3           , OS_KEY_3 },
            { KEY_4           , OS_KEY_4 },
            { KEY_5           , OS_KEY_5 },
            { KEY_6           , OS_KEY_6 },
            { KEY_7           , OS_KEY_7 },
            { KEY_8           , OS_KEY_8 },
            { KEY_9           , OS_KEY_9 },
            { KEY_SEMICOLON   , OS_KEY_SEMICOLON },
            { KEY_EQUAL       , OS_KEY_EQUAL },
            { KEY_A           , OS_KEY_A },
            { KEY_B           , OS_KEY_B },
            { KEY_C           , OS_KEY_C },
            { KEY_D           , OS_KEY_D },
            { KEY_E           , OS_KEY_E },
            { KEY_F           , OS_KEY_F },
            { KEY_G           , OS_KEY_G },
            { KEY_H           , OS_KEY_H },
            { KEY_I           , OS_KEY_I },
            { KEY_J           , OS_KEY_J },
            { KEY_K           , OS_KEY_K },
            { KEY_L           , OS_KEY_L },
            { KEY_M           , OS_KEY_M },
            { KEY_N           , OS_KEY_N },
            { KEY_O           , OS_KEY_O },
            { KEY_P           , OS_KEY_P },
            { KEY_Q           , OS_KEY_Q },
            { KEY_R           , OS_KEY_R },
            { KEY_S           , OS_KEY_S },
            { KEY_T           , OS_KEY_T },
            { KEY_U           , OS_KEY_U },
            { KEY_V           , OS_KEY_V },
            { KEY_W           , OS_KEY_W },
            { KEY_X           , OS_KEY_X },
            { KEY_Y           , OS_KEY_Y },
            { KEY_Z           , OS_KEY_Z },
            { KEY_LEFT_BRACKET   , OS_KEY_LEFT_BRACKET },
            { KEY_BACKSLASH      , OS_KEY_BACKSLASH },
            { KEY_RIGHT_BRACKET  , OS_KEY_RIGHT_BRACKET },
            { KEY_GRAVE_ACCENT   , OS_KEY_GRAVE_ACCENT },
            { KEY_ESCAPE         , OS_KEY_ESCAPE },
            { KEY_ENTER          , OS_KEY_ENTER },
            { KEY_TAB            , OS_KEY_TAB },
            { KEY_BACKSPACE      , OS_KEY_BACKSPACE },
            { KEY_INSERT         , OS_KEY_INSERT },
            { KEY_DELETE         , OS_KEY_DELETE },
            { KEY_RIGHT          , OS_KEY_RIGHT },
            { KEY_LEFT           , OS_KEY_LEFT },
            { KEY_DOWN           , OS_KEY_DOWN },
            { KEY_UP             , OS_KEY_UP },
            { KEY_PAGE_UP        , OS_KEY_PAGE_UP },
            { KEY_PAGE_DOWN      , OS_KEY_PAGE_DOWN },
            { KEY_HOME           , OS_KEY_HOME },
            { KEY_END            , OS_KEY_END },
            { KEY_CAPS_LOCK      , OS_KEY_CAPS_LOCK },
            { KEY_SCROLL_LOCK    , OS_KEY_SCROLL_LOCK },
            { KEY_NUM_LOCK       , OS_KEY_NUM_LOCK },
            { KEY_PRINT_SCREEN   , OS_KEY_PRINT_SCREEN },
            { KEY_PAUSE          , OS_KEY_PAUSE },
            { KEY_F1             , OS_KEY_F1 },
            { KEY_F2             , OS_KEY_F2 },
            { KEY_F3             , OS_KEY_F3 },
            { KEY_F4             , OS_KEY_F4 },
            { KEY_F5             , OS_KEY_F5 },
            { KEY_F6             , OS_KEY_F6 },
            { KEY_F7             , OS_KEY_F7 },
            { KEY_F8             , OS_KEY_F8 },
            { KEY_F9             , OS_KEY_F9 },
            { KEY_F10            , OS_KEY_F10 },
            { KEY_F11            , OS_KEY_F11 },
            { KEY_F12            , OS_KEY_F12 },
            { KEY_KP_0           , OS_KEY_KP_0 },
            { KEY_KP_1           , OS_KEY_KP_1 },
            { KEY_KP_2           , OS_KEY_KP_2 },
            { KEY_KP_3           , OS_KEY_KP_3 },
            { KEY_KP_4           , OS_KEY_KP_4 },
            { KEY_KP_5           , OS_KEY_KP_5 },
            { KEY_KP_6           , OS_KEY_KP_6 },
            { KEY_KP_7           , OS_KEY_KP_7 },
            { KEY_KP_8           , OS_KEY_KP_8 },
            { KEY_KP_9           , OS_KEY_KP_9 },
            { KEY_KP_PERIOD      , OS_KEY_KP_PERIOD },
            { KEY_KP_DIVIDE      , OS_KEY_KP_DIVIDE },
            { KEY_KP_MULTIPLY    , OS_KEY_KP_MULTIPLY },
            { KEY_KP_SUBTRACT    , OS_KEY_KP_SUBTRACT },
            { KEY_KP_ADD         , OS_KEY_KP_ADD },
            { KEY_KP_ENTER       , OS_KEY_KP_ENTER },
            { KEY_KP_EQUAL       , OS_KEY_KP_EQUAL },
            { KEY_LEFT_SHIFT     , OS_KEY_LEFT_SHIFT },
            { KEY_LEFT_CONTROL   , OS_KEY_LEFT_CONTROL },
            { KEY_LEFT_ALT       , OS_KEY_LEFT_ALT },
            { KEY_LEFT_SUPER     , OS_KEY_LEFT_SUPER },
            { KEY_RIGHT_SHIFT    , OS_KEY_RIGHT_SHIFT },
            { KEY_RIGHT_CONTROL  , OS_KEY_RIGHT_CONTROL },
            { KEY_RIGHT_ALT      , OS_KEY_RIGHT_ALT },
            { KEY_RIGHT_SUPER    , OS_KEY_RIGHT_SUPER },
    };

    OsKey Input::keyToOsKey(Key key) {
        return _keyMap[key];
    }

    Key Input::osKeyToKey(OsKey key) {
        auto it = find_if(_keyMap.begin(), _keyMap.end(),
                          [&key]( const auto &p )
                                {
                                    return p.second == key;
                                } );

        if (it != _keyMap.end()) return it->first;
        return KEY_NONE;
    }

#ifdef _WIN32
#include <Xinput.h>
#include <dinput.h>

    bool Input::_keys[256]{false};
    bool Input::_useXInput{false};

    struct _DirectInputState {
        LPDIRECTINPUTDEVICE8 device;
        DIJOYSTATE           state;
        string               name;
    };

    static map<uint32_t, _DirectInputState> _directInputStates{};
    static map<uint32_t, XINPUT_STATE> _xinputStates{};
    static LPDIRECTINPUT8 _directInput = nullptr;

    BOOL CALLBACK _enumGamepadsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
        if (_directInput) {
            _DirectInputState state {
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
            _directInputStates[_directInputStates.size()] = state;
        }
        return DIENUM_CONTINUE;
    }

    void Input::_initInput() {
        if (FAILED(DirectInput8Create(GetModuleHandle(nullptr),
                                      DIRECTINPUT_VERSION,
                                      IID_IDirectInput8,
                                      (void**)&_directInput, nullptr))) {
            die("DirectInput8Create failed");
        }
        _directInput->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                  _enumGamepadsCallback,
                                  nullptr,
                                  DIEDFL_ATTACHEDONLY);
    }

    void Input::_closeInput() {
        if (_directInput) {
            for (auto entry : _directInputStates) {
                entry.second.device->Release();
            }
            _directInputStates.clear();
            _directInput->Release();
            _directInput = nullptr;
        }
    }

    uint32_t Input::getConnectedJoypads() {
        uint32_t count = 0;
        /*if (_useXInput) {
            count += _xinputStates.size();
        } */
        if (_directInput) {
            count += _directInputStates.size();
        }
        return count;
    }

    bool Input::isGamepad(uint32_t index) {
        /*if (_useXInput) {
            if (_xinputStates.contains(index)) {
                XINPUT_CAPABILITIES xinputCapabilities;
                ZeroMemory(&xinputCapabilities, sizeof(XINPUT_CAPABILITIES));
                if (XInputGetCapabilities(index, 0, &xinputCapabilities) == ERROR_SUCCESS) {
                    return xinputCapabilities.SubType == XINPUT_DEVSUBTYPE_GAMEPAD;
                }
            }
        } */
        if (_directInput) {
            return _directInputStates.contains(index);
        }
        return false;
    }

    void Input::_updateInputStates() {
       /* if (_useXInput) {
            _xinputStates.clear();
            for (uint32_t i = 0; i < XUSER_MAX_COUNT; ++i) {
                XINPUT_STATE state;
                ZeroMemory(&state, sizeof(XINPUT_STATE));
                if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                    _xinputStates[i] = state;
                }
            }
        } */
       for (auto entry : _directInputStates) {
           if (FAILED(entry.second.device->GetDeviceState(sizeof(DIJOYSTATE), &entry.second.state))) {
               // remove from map
           }
           printf("X-axis: %ld\n", entry.second.state.lX);
           printf("Y-axis: %ld\n", entry.second.state.lY);
           printf("Buttons: ");
           for (int i = 0; i < 32; i++)
           {
               if (entry.second.state.rgbButtons[i] & 0x80)
               {
                   printf("%d ", i);
               }
           }
           printf("\n");
       }
    }

    string Input::getGamepadName(uint32_t index) {
        /*if (_useXInput) {
            return "XInput";
        }*/
        if (_directInputStates.contains(index)){
            return _directInputStates[index].name;
        }
        return "??";
    }

    vec2 Input::getKeyboardVector(Key keyNegX, Key keyPosX, Key keyNegY, Key keyPosY) {
        auto x = _keys[keyToOsKey(keyNegX)] ? -1 : _keys[keyToOsKey(keyPosX)] ? 1 : 0;
        auto y = _keys[keyToOsKey(keyNegY)] ? -1 : _keys[keyToOsKey(keyPosY)] ? 1 : 0;
        vec2 vector{ x, y };
        float length = glm::length(vector);
        return (length > 1.0f) ? vector / length : vector;
    }

    void Input::setMouseMode(MouseMode mode) {
        auto& wnd = Application::get().getWindow();
        MSG msg;
        while(PeekMessageW(&msg, wnd._getHandle(), 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        switch (mode) {
            case MOUSE_MODE_VISIBLE:
                ReleaseCapture();
                ClipCursor(nullptr);
                ShowCursor(TRUE);
                SetCursorPos(wnd._getRect().left + wnd.getWidth() / 2,
                             wnd._getRect().top + wnd.getHeight() / 2);
                break;
            case MOUSE_MODE_HIDDEN:
                ReleaseCapture();
                ClipCursor(nullptr);
                ShowCursor(FALSE);
                break;
            case MOUSE_MODE_VISIBLE_CAPTURED: {
                auto rect = wnd._getRect();
                SetCapture(wnd._getHandle());
                ClipCursor(&rect);
                ShowCursor(TRUE);
                SetCursorPos(wnd._getRect().left + wnd.getWidth() / 2,
                             wnd._getRect().top + wnd.getHeight() / 2);
                break;
            }
            case MOUSE_MODE_HIDDEN_CAPTURED: {
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