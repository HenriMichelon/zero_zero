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
            { KEY_A           , OS_KEY_1 },
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
    void Input::setMouseMode(MouseMode mode) {
        auto& wnd = Application::get().getWindow();
        MSG msg;
        while(PeekMessageW(&msg, wnd._getHandle(), 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        SetCursorPos(wnd._getRect().left + wnd.getWidth() / 2,
                     wnd._getRect().top + wnd.getHeight() / 2);
        switch (mode) {
            case MOUSE_MODE_VISIBLE:
                ReleaseCapture();
                ShowCursor(TRUE);
                ClipCursor(nullptr);
                break;
            case MOUSE_MODE_HIDDEN:
                ReleaseCapture();
                ShowCursor(FALSE);
                ClipCursor(nullptr);
                break;
            case MOUSE_MODE_VISIBLE_CAPTURED: {
                auto rect = wnd._getRect();
                SetCapture(wnd._getHandle());
                ClipCursor(&rect);
                ShowCursor(TRUE);
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