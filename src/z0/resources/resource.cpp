module;
#include "z0/libraries.h"

module z0;

import :Resource;

namespace z0 {

    Resource::Resource(string name):
        name{std::move(name)},
        id{currentId++} {
        replace(name.begin(), name.end(), '/', '_');
        replace(name.begin(), name.end(), '\\', '_');
        replace(name.begin(), name.end(), ':', '_');
    }

    Resource::id_t Resource::currentId = 0;

}
