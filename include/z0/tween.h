#pragma once


namespace z0 {

    /**
     * Base class for all tweeners classes
     */
    class Tween: public Object {
    public:
        /**
         * Update the tween.
         * If the Tween have been created manually you need to call update() in your Node::onPhysicsProcess() function.
         * Do not call it if the Tween have been created with .
         * * @return `false` if the tween is running
         */
        virtual bool update(float deltaTime) = 0;

        bool isRunning() const { return running; }

    protected:
        bool running{false};
    };

    /**
     * Tweens are objects that perform a specific animating task, e.g. interpolating a property of a Node. 
     */
    template<typename T>
    class PropertyTween: public Tween {
    public:
        /**
         * A Setter method will be called by the Tween each update
         */
        typedef void (Object::*Setter)(T);

        /**
         * Create a Tween to tweens a property of an `node` between an `initial` value 
         * and `final` value in a span of time equal to `duration`, in seconds.
         */
        PropertyTween(Object* node, Setter set, T initial, T final, float duration):
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{node},
            setter{set} {}

         /**
         * Create a Tween to tweens a property of an `node` between an `initial` value 
         * and `final` value in a span of time equal to `duration`, in seconds.
         */
        PropertyTween(const shared_ptr<Object>& node, Setter set, T initial, T final, float duration):
            durationTime{duration},
            targetValue{final},
            startValue{initial},
            targetObject{node.get()},
            setter{set} {}

        /**
         * Interpolate the property.
         * If the Tween have been created manually you need to call update() in your Node::onPhysicsProcess() function.
         * Do not call it if the Tween have been created with .
         * @return `false` if the tween is running
         */
        bool update(float deltaTime) override {
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