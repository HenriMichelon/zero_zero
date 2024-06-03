#include "z0/z0.h"
#include "z0/resources/resource.h"

namespace z0 {

    Resource::id_t Resource::currentId = 0;

    Resource::Resource(string  resName):
        name{std::move(resName)},
        id{currentId++} {
        replace(name.begin(), name.end(),  '/', '_');
    }

    void Resource::_incrementReferenceCounter() {
        refCount += 1;
    }

    bool Resource::_decrementReferenceCounter() {
        refCount -= 1;
        return refCount == 0;
    }

}