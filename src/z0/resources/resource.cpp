#include "z0/resources/resource.h"

#include <algorithm>

namespace z0 {

    Resource::id_t Resource::currentId = 0;

    Resource::Resource(const string& resName):
        name{resName},
        id{currentId++} {
        replace(name.begin(), name.end(),  '/', '_');
    }

}