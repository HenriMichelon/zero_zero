module;

module z0;

namespace z0 {

    void Signal::emit(Parameters *params) const {
        for (const auto &callable : handlers) {
            (callable.obj->*callable.func)(params);
        }
    }

}
