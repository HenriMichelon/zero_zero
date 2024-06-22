#pragma once

namespace z0 {

    class Object;

    /**
     * A signal of an Object
     */
    class Signal {
    public:
        using signal = std::string;

        /**
         * Base struct for emit() parameters
         */
        struct Parameters {};

        /**
         * Callable member function
         */
        typedef void (Object::*Handler)(Parameters*);
        
        /**
         * Connects a member function to the signal
         * @param object object containing the member function to connect
         * @param handler the member function to call when emit() is called
        */
        void connect(Object* object, Handler handler);

        /**
         * Disconnects a member function to the signal
         * @param object object containing the member function to connect
         * @param handler the member function to call when emit() is called
        */
        void disconnect(Object* object, Handler handler);

        /**
         * Emits the signal by calling all the connected functions in the connect order
         * @param name signal name
         * @param params parameters to pass to the function connected to the signal
         */
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