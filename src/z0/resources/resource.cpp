#include "z0/resources/resource.h"

#include <algorithm>
#include <utility>

namespace z0 {

    Resource::id_t Resource::currentId = 0;

    Resource::Resource(string  resName):
        name{std::move(resName)},
        id{currentId++} {
        replace(name.begin(), name.end(),  '/', '_');
    }

}