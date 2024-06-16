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
        inline InputEventType getType() const { return type; }

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
        inline Key getKey() const { return keycode; }

        /**
         * The repeat count for the current event. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key
         */
        int getRepeatCount() const { return repeat; }

        /**
         * Returns true if the key is pressed
         */
        inline bool isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
         */
        inline int getModifiers() const { return modifiers; }

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
        inline GamepadButton getGamepadButton() const { return button; }

         /**
         * Returns true if the gamepad button is pressed
         */
        inline bool isPressed() const { return pressed; }

    private:
        GamepadButton button;
        bool pressed;
    };

    /** 
     * Mouse move event
    */
    class InputEventMouseMotion: public InputEvent {
    public:
        InputEventMouseMotion(float posX, float posY, float relativeX, float relativeY);

        /**
         * Returns the current mouse position
         */
        inline vec2 getPosition() const { return vec2{x, y}; }

        /**
         * Returns the current mouse x position
         */
        inline float getX() const { return x; }

        /**
         * Returns the current mouse y position
         */
        inline float getY() const { return y; }

        /**
         * Returns the relative x mouvement
         */
        inline float getRelativeX() const { return relativeX; }

        /**
         * Returns the relative y mouvement
         */
        inline float getRelativeY() const { return relativeY; }

    private:
        float x, y;
        float relativeX, relativeY;
    };

    /** 
     * Mouse button pressed/released event
    */
    class InputEventMouseButton: public InputEvent {
    public:
        InputEventMouseButton(MouseButton button, bool pressed, int modifiers, float posX, float posY);

        /**
         * Returns the mouse button
         */
        inline MouseButton getMouseButton() const { return button; }

        /**
         * Returns true is the button is pressed
         */
        inline bool isPressed() const { return pressed; }

        /**
         * Returns the state of the z0::KeyModifier keys
        */
        inline int getModifiers() const { return modifiers; }

        /**
         * Returns the current mouse position
         */
        inline vec2 getPosition() const { return vec2{x, y}; }

        /**
         * Returns the current mouse x position
         */
        inline float getX() const { return x; }

        /**
         * Returns the relative y mouvement
         */
        inline float getY() const { return y; }

    private:
        float x, y;
        MouseButton button;
        bool pressed;
        int modifiers;
    };

}