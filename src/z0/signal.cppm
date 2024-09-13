module;
#include "z0/libraries.h"

export module Z0:Signal;

export namespace z0 {

    class Object;

    /**
     * A signal of an Object
     */
    class Signal {
    public:
        using signal = string;

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
        void connect(Object* object, const Handler handler) {
            handlers.push_back(SignalCallable{object, handler});
        }

        /**
         * Disconnects a member function to the signal
         * @param object object containing the member function to connect
         * @param handler the member function to call when emit() is called
        */
        void disconnect(Object* object, const Handler handler) {
            handlers.remove(SignalCallable{object, handler});
        }
        /**
         * Emits the signal by calling all the connected functions in the connect order
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(Parameters* params) const {
            for (const auto& callable : handlers) {
                (callable.obj->*callable.func)(params);
            }
        }
        
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