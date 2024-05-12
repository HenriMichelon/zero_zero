#pragma once

#include "z0/object.h"

namespace z0 {

    class InputEvent: public Object {
    public:
        InputEventType getType() const { return type; }

    protected:
        explicit InputEvent(InputEventType _type): type{_type} {};

    private:
        InputEventType type;
    };

    class InputEventKey: public InputEvent {
    public:
        InputEventKey(Key key, bool pressed, int repeat, int modifiers);

        Key getKeyCode() const { return keycode; }
        int getRepeatCount() const { return repeat; }
        bool isPressed() const { return pressed; }
        int getModifiers() const { return modifiers; }

    private:
        Key keycode;
        int repeat;
        bool pressed;
        int modifiers;
    };

    class InputEventMouseButton: public InputEvent {
    public:
        InputEventMouseButton(MouseButton button, bool pressed, int modifiers, float posX, float posY);

        MouseButton getMouseButton() const { return button; }
        bool isPressed() const { return pressed; }
        int getModifiers() const { return modifiers; }
        vec2 getPosition() const { return vec2{x, y}; }
        float getX() const { return x; }
        float getY() const { return y; }

    private:
        float x, y;
        MouseButton button;
        bool pressed;
        int modifiers;
    };

    class InputEventMouseMotion: public InputEvent {
    public:
        InputEventMouseMotion(float posX, float posY, float relativeX, float relativeY);

        vec2 getPosition() const { return vec2{x, y}; }
        float getX() const { return x; }
        float getY() const { return y; }
        float getRelativeX() const { return relativeX; }
        float getRelativeY() const { return relativeY; }

    private:
        float x, y;
        float relativeX, relativeY;
    };

}