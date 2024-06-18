#include <z0/z0.h>
#ifndef USE_PCH
#include "signal.h"
#endif

namespace z0 {

    void Signal::connect(Object* object, Handler handler) {
        handlers.push_back(SignalCallable{object, handler});
    }

    void Signal::disconnect(Object* object, Handler handler) {
        handlers.remove(SignalCallable{object, handler});
    }
    
    void Signal::emit(Parameters* params) {
        for (const auto& callable : handlers) {
            (callable.obj->*callable.func)(params);            
        }
    }

}