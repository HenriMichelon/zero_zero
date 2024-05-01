#include "z0/input.h"

#include <unordered_map>

namespace z0 {

    list<shared_ptr<InputEvent>> Input::_inputQueue;
    unordered_map<Key, bool> Input::_keyPressedStates;
    unordered_map<Key, bool> Input::_keyJustPressedStates;
    unordered_map<Key, bool> Input::_keyJustReleasedStates;

    void Input::injectInputEvent(const shared_ptr<InputEvent>& event) {
        _inputQueue.push_back(event);
    }

    shared_ptr<InputEvent> Input::consumeInputEvent() {
        const auto& event = _inputQueue.front();
        _inputQueue.pop_front();
        return event;
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
}