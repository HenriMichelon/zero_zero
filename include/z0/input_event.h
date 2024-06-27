#pragma once

namespace z0 {

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
        explicit InputEvent(InputEventType _type): type{_type} {};

    private:
        InputEventType type;
    };

    /**
     * Keyboard input event
     */
    class InputEventKey: public InputEvent {
    public:
        InputEventKey(Key key, bool pressed, int repeat, int modifiers);

        /**
         * Returns the key code
         */
        [[nodiscard]] inline Key getKey() const { return keycode; }

        /**
         * The repeat count for the current event. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key
         */
        [[nodiscard]] int getRepeatCount() const { return repeat; }

        /**
         * Returns true if the key is pressed
         */
        [[nodiscard]] inline bool isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
         */
        [[nodiscard]] inline int getModifiers() const { return modifiers; }

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
        InputEventGamepadButton(GamepadButton button, bool pressed);

        /**
         * Return the gamepad button
         */
        [[nodiscard]] inline GamepadButton getGamepadButton() const { return button; }

         /**
         * Returns true if the gamepad button is pressed
         */
        [[nodiscard]] inline bool isPressed() const { return pressed; }

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
        [[nodiscard]] inline vec2 getPosition() const { return vec2{x, y}; }

        /**
         * Returns the current mouse x position
         */
        [[nodiscard]] inline float getX() const { return x; }

        /**
         * Returns the current mouse y position
         */
        [[nodiscard]] inline float getY() const { return y; }

        /**
         * Returns the mouse button states (which button is down)
         */
        [[nodiscard]] inline uint32_t getButtonsState() { return buttonsState; }

    protected:
        InputEventMouse(InputEventType type, uint32_t buttonsState, float posX, float posY);

    private:
        float x, y;
        uint32_t buttonsState;
    };


    /** 
     * Mouse move event
    */
    class InputEventMouseMotion: public InputEventMouse {
    public:
        InputEventMouseMotion(uint32_t buttonsState, float posX, float posY, float relativeX, float relativeY);

        /**
         * Returns the relative x mouvement
         */
        [[nodiscard]] inline float getRelativeX() const { return relativeX; }

        /**
         * Returns the relative y mouvement
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
        InputEventMouseButton(MouseButton button, bool pressed, int modifiers, uint32_t buttonsState, float posX, float posY);

        /**
         * Returns the mouse button
         */
        [[nodiscard]] inline MouseButton getMouseButton() const { return button; }

        /**
         * Returns true is the button is pressed
         */
        [[nodiscard]] inline bool isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
        */
        [[nodiscard]] inline int getModifiers() const { return modifiers; }

    private:
        MouseButton button;
        bool pressed;
        int modifiers;
    };

}