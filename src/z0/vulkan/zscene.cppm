/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:VulkanZScene;

import :ZScene;

export namespace z0 {

    class VulkanZScene : public ZScene {
    protected:
        void loadImages(ifstream& stream) override;
    };

}
