#include "z0/resources/resource.h"

#include <algorithm>

namespace z0 {

    Resource::id_t Resource::currentId = 0;

    Resource::Resource(string resName): name{move(resName)}, id{currentId++}   {
        replace(name.begin(), name.end(),  '/', '_');
    }

}