#pragma once

namespace z0 {

    class Object;

    class Signal {
    public:
        struct Parameters {};
        typedef void (Object::*Handler)(Parameters*);
        
        void connect(Object* object, Handler handler);
        void disconnect(Object* object, Handler handler);
        void emit(Parameters* param);

    private:
         struct SignalCallable {
            Object* obj{nullptr};
            Handler func{nullptr};

            inline bool operator==(const SignalCallable& other) const { 
                return (obj == other.obj) && (func == other.func);
            }
        };
        list<SignalCallable> handlers;
    };

}