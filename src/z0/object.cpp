#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/signal.h"
#endif

namespace z0 {

    map<string, Signal> Object::signals;

    void Object::connect(const string& name, Object* object, Signal::Handler handler) {
        signals[name].connect(object, handler);
    }

    void Object::disconnect(const string& name, Object* object, Signal::Handler handler) {
        if (signals.contains(name)) {
            signals[name].disconnect(object, handler);
        }
    }

    void Object::emit(const string& name, Signal::Parameters* params) {
        if (signals.contains(name)) {
            signals[name].emit(params);
        }
    }
    
}