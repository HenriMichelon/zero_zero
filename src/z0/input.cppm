module;
#ifdef _WIN32
#include <Xinput.h>
#include <dinput.h>
#endif
#include "z0/libraries.h"

export module Z0:Input;

import :Constants;
import :Tools;
import :InputEvent;

namespace z0 {
    /**
     * A singleton for handling inputs
     */
    export class Input {
    public:
        /**
         * Returns true if you are pressing the key
         */
        [[nodiscard]] static bool isKeyPressed(const Key key) {
            return _keyPressedStates[key];
        }

        /**
         * Returns true when the user has started pressing the key
         */
        [[nodiscard]] static bool isKeyJustPressed(const Key key) {
            const auto result = _keyJustPressedStates[key];
            _keyJustPressedStates[key] = false;
            return result;
        }

        /**
         * Returns true when the user stops pressing the key
         */
        [[nodiscard]] static bool isKeyJustReleased(const Key key) {
            const auto result = _keyJustReleasedStates[key];
            _keyJustReleasedStates[key] = false;
            return result;
        }

        /**
         * Gets an input vector by specifying four keys for the positive and negative X and Y axes.
         */
        [[nodiscard]] static vec2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        /**
         * Returns true if you are pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonPressed(const MouseButton mouseButton) {
            return _mouseButtonPressedStates[mouseButton];
        }

        /**
         * Returns true when the user has started pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonJustPressed(const MouseButton mouseButton) {
            const auto result = _mouseButtonJustPressedStates[mouseButton];
            _mouseButtonJustPressedStates[mouseButton] = false;
            return result;
        }

        /**
         * Returns true when the user stops pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonJustReleased(const MouseButton mouseButton) {
            const auto result = _mouseButtonJustReleasedStates[mouseButton];
            _mouseButtonJustReleasedStates[mouseButton] = false;
            return result;
        }

        /**
         * Sets the mouse visibility and capture mode
         */
        static void setMouseMode(MouseMode mode);

        /**
         * Sets the mouse cursor
         */
        static void setMouseCursor(MouseCursor cursor);

        /**
         * Returns the number of connected joypads, including gamepads
         */
        [[nodiscard]] static uint32_t getConnectedJoypads();

        /**
         * Returns true if the joypad is a gamepad
         * @param index index of the joypad in [0..getConnectedJoypads()]
         */
        [[nodiscard]] static bool isGamepad(uint32_t index);

        /**
         * Returns the joypad name
         * @param index index of the joypad in [0..getConnectedJoypads()]
         */
        [[nodiscard]] static string getJoypadName(uint32_t index);

        /**
         * Gets an input vector for a gamepad joystick
         * @param index index of the joypad in [0..getConnectedJoypads()]
         * @param axisJoystick axis
         */
        [[nodiscard]] static vec2 getGamepadVector(uint32_t index, GamepadAxisJoystick axisJoystick);

        /**
        * Returns true if you are pressing the gamepad button
        * @param index index of the joypad in [0..getConnectedJoypads()]
        * @param gamepadButton gamepad button
        */
        [[nodiscard]] static bool isGamepadButtonPressed(uint32_t index, GamepadButton gamepadButton);
        //static float getGamepadAxisValue(uint32_t index, GamepadAxis gamepadAxis);

    private:
        [[nodiscard]] static float applyDeadzone(float value, float deadzonePercent);
        static void generateGamepadButtonEvent(GamepadButton, bool);

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
        static map<MouseCursor, HCURSOR> _mouseCursors;
        static const int DI_AXIS_RANGE;
        static const float DI_AXIS_RANGE_DIV;
        static bool _keys[256];
        static bool _useXInput;
        static void _initInput();
        static void _closeInput();
        static void _updateInputStates();
#endif
    };
}
