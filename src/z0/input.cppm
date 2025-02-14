/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <dinput.h>
#endif
#include "z0/libraries.h"

export module z0.Input;

import z0.Constants;

namespace z0 {
    /**
     * %A singleton for handling inputs
     */
    export class Input {
    public:
        /**
         * Returns true if you are pressing the key
         */
        [[nodiscard]] static bool isKeyPressed(Key key);

        /**
         * Returns true when the user has started pressing the key
         */
        [[nodiscard]] static bool isKeyJustPressed(Key key);

        /**
         * Returns true when the user stops pressing the key
         */
        [[nodiscard]] static bool isKeyJustReleased(Key key);

        /**
         * Gets an input vector by specifying four keys for the positive and negative X and Y axes.
         */
        [[nodiscard]] static vec2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        /**
         * Returns true if you are pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonPressed(MouseButton mouseButton);

        /**
         * Returns true when the user has started pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonJustPressed(MouseButton mouseButton);
        /**
         * Returns true when the user stops pressing the mouse button
         */
        [[nodiscard]] static bool isMouseButtonJustReleased(MouseButton mouseButton);

        /**
         * Sets the mouse visibility and capture mode
         */
        static void setMouseMode(MouseMode mode);

        /**
         * Sets the mouse cursor
         */
        static void setMouseCursor(MouseCursor cursor);

        /**
         * Sets the mouse position to the center of the window
         */
        static void resetMousePosition();

        /**
         * Returns the mouse position
         */
        static vec2 getMousePosition();

        /**
         * Returns the mouse position
         */
        static void setMousePosition(const vec2& position);

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

        static bool isGamepadButtonJustReleased(GamepadButton button);
        static bool isGamepadButtonJustPressed(GamepadButton button);

    private:
        [[nodiscard]] static float applyDeadzone(float value, float deadzonePercent);
        static void generateGamepadButtonEvent(GamepadButton, bool);

    public:
        static unordered_map<Key, bool> _keyPressedStates;
        static unordered_map<Key, bool> _keyJustPressedStates;
        static unordered_map<Key, bool> _keyJustReleasedStates;
        static unordered_map<MouseButton, bool> _mouseButtonPressedStates;
        static unordered_map<MouseButton, bool> _mouseButtonJustPressedStates;
        static unordered_map<MouseButton, bool> _mouseButtonJustReleasedStates;
        static unordered_map<GamepadButton, bool> _gamepadButtonPressedStates;
        static unordered_map<GamepadButton, bool> _gamepadButtonJustPressedStates;
        static unordered_map<GamepadButton, bool> _gamepadButtonJustReleasedStates;

        static OsKey keyToOsKey(Key key);
        static Key osKeyToKey(OsKey key);

#ifdef _WIN32
        static map<MouseCursor, HCURSOR> _mouseCursors;
        static const int DI_AXIS_RANGE;
        static const float DI_AXIS_RANGE_DIV;
        static bool _useXInput;
        static void _initInput();
        static void _closeInput();
        static void _updateInputStates();
#endif
    };
}
