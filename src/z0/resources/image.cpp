#include "z0/resources/image.h"

namespace z0 {

    Image::Image(const filesystem::path& filename): Resource{filename.string()} {
    }

}