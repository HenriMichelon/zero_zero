/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Tween;

import z0.Object;
import z0.Constants;
import z0.Tools;

export namespace z0 {

    /**
     * Base class for all tweeners classes.<br>
     * Tweens are objects that perform a specific animating task, e.g. interpolating a property of an Object. 
     */
    class Tween: public Object {
    public:
        /**
         * Update the tween.
         * If the Tween have been created manually you need to call update() in your Node::onPhysicsProcess() function.
         * Do not call it if the Tween have been created with Node::create*Tween().
         * * @return `false` if the tween is running
         */
        [[nodiscard]] virtual bool update(float deltaTime) = 0;

        [[nodiscard]] bool isRunning() const { return running; }

    protected:
        bool running{false};
        TransitionType interpolationType;

        explicit Tween(TransitionType type): interpolationType{type} {};

    public:
        void _kill() { running = false; }
    };

    /**
     * Tween to interpolate a property of an Object.
     */
    template<typename T>
    class PropertyTween: public Tween {
    public:
        /**
         * A Setter method will be called by the Tween each update
         */
        typedef void (Object::*Setter)(T);

        /**
         * Create a Tween to tweens a property of an Object.
         * @param obj Target Object
         * @param set Setter to call on the Object
         * @param initial Initial value
         * @param final Final value
         * @param duration Animation duration in seconds
         * @param ttype Transition type
         */
        PropertyTween(Object* obj,
                      const Setter set,
                      T initial, 
                      T final, 
                      const float duration,
                      const TransitionType ttype = TRANS_LINEAR):
            Tween{ttype},
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{obj},
            setter{set} {}

         /**
         * Create a Tween to tweens a property of an Object.
         * @param obj Target Object
         * @param set Setter to call on the Object
         * @param initial Initial value
         * @param final Final value
         * @param duration Animation duration in seconds
         * @param ttype Transition type
         */
        PropertyTween(const shared_ptr<Object>& obj, 
                      const Setter set,
                      T initial, 
                      T final, 
                      const float duration,
                      const TransitionType ttype = TRANS_LINEAR):
            Tween{ttype},
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{obj.get()},
            setter{set} {}

        /**
         * Interpolate the property.
         * If the Tween have been created manually you need to call update() in a Node::onPhysicsProcess() function.
         * *Do not call it* if the Tween have been created with Node::createPropertyTween().
         * @return `false` if the tween is running
         */
        [[nodiscard]] bool update(const float deltaTime) override {
            elapsedTime += deltaTime;
            float t = std::min(elapsedTime / durationTime, 1.0f); // Normalized time
            (targetObject->*setter)(lerp(startValue, targetValue, t));
            running = (t < 1.0);
            return !running;
        }

    private:
        float durationTime;
        float elapsedTime{0.0f};
        T targetValue;
        T startValue;
        Object* targetObject;
        Setter setter;
    };

}