/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.InputEvent;

import z0.Constants;
import z0.Object;

export namespace z0 {

    /**
     * Base class of all input events
     */
    class InputEvent: public Object {
    public:
        /**
         * Returns the type of the event
         */
        [[nodiscard]] inline InputEventType getType() const { return type; }

    protected:
        explicit InputEvent(const InputEventType _type): type{_type} {}

    private:
        InputEventType type;
    };

    /**
     * Keyboard input event
     */
    class InputEventKey: public InputEvent {
    public:
         InputEventKey(const Key _key, const bool _pressed, const int _repeat, const int _modifiers):
             InputEvent{InputEventType::KEY},
             keycode{_key},
             repeat{_repeat},
             pressed{_pressed},
             modifiers{_modifiers} {}

        /**
         * Returns the key code
         */
        [[nodiscard]] inline Key getKey() const { return keycode; }

        /**
         * The repeat count for the current event. The value is the number of times the keystroke is auto-repeated as a result of the user holding down the key
         */
        [[nodiscard]] auto getRepeatCount() const { return repeat; }

        /**
         * Returns true if the key is pressed
         */
        [[nodiscard]] inline auto isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
         */
        [[nodiscard]] inline auto getModifiers() const { return modifiers; }

    private:
        Key keycode;
        int repeat;
        bool pressed;
        int modifiers;
    };

    /**
     * Gamepad buttons event
     */
    class InputEventGamepadButton: public InputEvent {
    public:
        InputEventGamepadButton(const GamepadButton _button, const bool _pressed):
            InputEvent{InputEventType::GAMEPAD_BUTTON},
            button{_button},
            pressed{_pressed} {}

        /**
         * Return the gamepad button
         */
        [[nodiscard]] inline auto getGamepadButton() const { return button; }

         /**
         * Returns true if the gamepad button is pressed
         */
        [[nodiscard]] inline auto isPressed() const { return pressed; }

    private:
        GamepadButton button;
        bool pressed;
    };

    /**
     * Base mouse event
    */
    class InputEventMouse: public InputEvent {
    public:
        /**
         * Returns the current mouse position
         */
        [[nodiscard]] inline auto getPosition() const { return vec2{x, y}; }

        /**
         * Returns the current mouse x position
         */
        [[nodiscard]] inline auto getX() const { return x; }

        /**
         * Returns the current mouse y position
         */
        [[nodiscard]] inline auto getY() const { return y; }

        /**
         * Returns the mouse button states (which button is down)
         */
        [[nodiscard]] inline auto getButtonsState() { return buttonsState; }

        /**
         * Returns the state of the z0::KeyModifier keys
        */
        [[nodiscard]] inline auto getModifiers() const { return modifiers; }

    protected:
        InputEventMouse(const InputEventType type, const uint32_t _buttonsState, const int _modifiers, const float posX, const float posY):
            InputEvent{type},
            x{posX},
            y{posY},
            buttonsState{_buttonsState},
            modifiers{_modifiers} {}

    private:
        float x, y;
        uint32_t buttonsState;
        int modifiers;
    };


    /**
     * Mouse move event
    */
    class InputEventMouseMotion: public InputEventMouse {
    public:
        InputEventMouseMotion(const uint32_t buttonsState, const int modifiers, const float posX, const float posY, const float rX, const float rY):
            InputEventMouse{InputEventType::MOUSE_MOTION, buttonsState, modifiers, posX, posY},
            relativeX{rX},
            relativeY{rY} {}

        /**
         * Returns the relative x movement
         */
        [[nodiscard]] inline float getRelativeX() const { return relativeX; }

        /**
         * Returns the relative y movement
         */
        [[nodiscard]] inline float getRelativeY() const { return relativeY; }

    private:
        float relativeX, relativeY;
    };

    /**
     * Mouse button pressed/released event
    */
    class InputEventMouseButton: public InputEventMouse {
    public:
        InputEventMouseButton(const MouseButton _button, const bool _pressed,const  int modifiers, const uint32_t buttonsState, const float posX, const float posY):
            InputEventMouse{InputEventType::MOUSE_BUTTON, buttonsState, modifiers, posX, posY},
            button{_button},
            pressed{_pressed} {}

        /**
         * Returns the mouse button
         */
        [[nodiscard]] inline MouseButton getMouseButton() const { return button; }

        /**
         * Returns true is the button is pressed
         */
        [[nodiscard]] inline bool isPressed() const { return pressed; }

    private:
        MouseButton button;
        bool pressed;
    };

}