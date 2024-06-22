#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/signal.h"
#endif

namespace z0 {

    void Object::connect(const Signal::signal& name, Object* object, Signal::Handler handler) {
        signals[name].connect(object, handler);
    }

    void Object::disconnect(const Signal::signal& name, Object* object, Signal::Handler handler) {
        if (signals.contains(name)) {
            signals[name].disconnect(object, handler);
        }
    }

    void Object::emit(const Signal::signal& name, Signal::Parameters* params) {
        if (signals.contains(name)) {
            signals[name].emit(params);
        }
    }
    
}