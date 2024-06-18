#pragma once

namespace z0 {

    /**
     * Base class for anything
     */
    class Object {
    public:
        /**
         * Connects a signal by name to a member function
         * @param name signal name.
         * @param object object containing the member function to connect
         * @param handler the member function to call when emit() is called
        */
        void connect(const string& name, Object* object, Signal::Handler handler);

        /**
         * Disconnects a signal by name from a member function
         * @param name signal name.
         * @param object object containing the member function to disconnect
         * @param handler the member function to call when emit() is called
        */
        void disconnect(const string& name, Object* object, Signal::Handler handler);

        /**
         * Emits a signal by name by calling all the connected functions in the connect order
         * @param name signal name
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(const string& name, Signal::Parameters* params = nullptr);

        /**
         * Converts the objet to a readable text
         */
        virtual string toString() const { return "??"; };

        friend ostream& operator<<(ostream& os, const Object& obj) {
            os << obj.toString();
            return os;
        }

    private:
        map<string, Signal> signals;
    };

}