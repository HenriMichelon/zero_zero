#pragma once

#include "z0/resources/resource.h"

#include <filesystem>

namespace z0 {

    class Image: public Resource {
    public:
        explicit Image(const filesystem::path& filename);

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        //glm::vec2 getSize() const;

    private:

    public:
    };

}