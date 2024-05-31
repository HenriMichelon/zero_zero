#pragma once

namespace z0 {

    class Input {
    public:
        static bool isKeyPressed(Key key);
        static bool isKeyJustPressed(Key key);
        static bool isKeyJustReleased(Key key);
        static vec2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        static bool isMouseButtonPressed(MouseButton mouseButton);
        static bool isMouseButtonJustPressed(MouseButton mouseButton);
        static bool isMouseButtonJustReleased(MouseButton mouseButton);
        static void setMouseMode(MouseMode mode);

        static uint32_t getConnectedJoypads();
        static bool isGamepad(uint32_t index);
        static string getGamepadName(uint32_t index);
        static vec2 getGamepadVector(uint32_t index, GamepadAxisJoystick axisJoystick);
        static bool isGamepadButtonPressed(uint32_t index, GamepadButton gamepadButton);
        //static float getGamepadAxisValue(uint32_t index, GamepadAxis gamepadAxis);

        /*static bool haveInputEvent() { return !_inputQueue.empty(); }
        static shared_ptr<InputEvent> consumeInputEvent();
        static void injectInputEvent(const shared_ptr<InputEvent>& event);*/

    public:
        //static list<shared_ptr<InputEvent>> _inputQueue;
        static unordered_map<Key, bool> _keyPressedStates;
        static unordered_map<Key, bool> _keyJustPressedStates;
        static unordered_map<Key, bool> _keyJustReleasedStates;
        static unordered_map<MouseButton, bool> _mouseButtonPressedStates;
        static unordered_map<MouseButton, bool> _mouseButtonJustPressedStates;
        static unordered_map<MouseButton, bool> _mouseButtonJustReleasedStates;
        static unordered_map<GamepadButton, bool> _gamepadButtonPressedStates;

        static OsKey keyToOsKey(Key key);
        static Key osKeyToKey(OsKey key);

#ifdef _WIN32
        static const int   DI_AXIS_RANGE;
        static const float DI_AXIS_RANGE_DIV;
        static bool _keys[256];
        static bool _useXInput;
        static void _initInput();
        static void _closeInput();
        static void _updateInputStates();
#endif

    private:
        static const float DEADZONE_PERCENT;
        static float applyDeadzone(float value);
    };


}