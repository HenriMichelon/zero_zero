#pragma once

#include "z0/input_event.h"

#include <list>
#include <unordered_map>

namespace z0 {

    class Input {
    public:
        static bool isKeyPressed(Key key);
        static bool isKeyJustPressed(Key key);
        static bool isKeyJustReleased(Key key);
        //static vec2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        //static bool isMouseButtonPressed(MouseButton mouseButton);
        static void setMouseMode(MouseMode mode);
/*
        static int getConnectedJoypads();
        static bool isGamepad(int index);
        static string getGamepadName(int index);
        static bool isGamepadButtonPressed(int index, GamepadButton gamepadButton);
        static float getGamepadAxisValue(int index, GamepadAxis gamepadAxis);
        static vec2 getGamepadVector(int index, GamepadAxisJoystick axisJoystick);*/

        static bool haveInputEvent() { return !_inputQueue.empty(); }
        static shared_ptr<InputEvent> consumeInputEvent();
        static void injectInputEvent(const shared_ptr<InputEvent>& event);

    public:
        static list<shared_ptr<InputEvent>> _inputQueue;
        static unordered_map<Key, bool> _keyPressedStates;
        static unordered_map<Key, bool> _keyJustPressedStates;
        static unordered_map<Key, bool> _keyJustReleasedStates;

        static OsKey keyToOsKey(Key key);
        static Key osKeyToKey(OsKey key);
    };


}