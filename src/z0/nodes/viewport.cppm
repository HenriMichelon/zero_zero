/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.Viewport;

import z0.nodes.Node;

export namespace z0 {

    /**
     * Node that define the scene renderer viewport position & size<br>
     * If no Viewport is defined in the scene the whole rendering Window surface is used
     */
    class Viewport : public Node {
    public:
        /**
         * Creates a Viewport
         * @param position top left viewport position in pixels
         * @param size size in pixels
         * @param name node name
         */
        Viewport(const vec2 &position, const vec2 &size, const string &name = TypeNames[VIEWPORT]);

        explicit Viewport(const string &name = TypeNames[VIEWPORT]);

        /**
         * Sets the viewport size in pixels
         */
        [[nodiscard]] inline const auto& getViewportSize() const { return size; }

        /**
         * Sets the top left viewport position in pixels
         */
        [[nodiscard]] inline const auto& getViewportPosition() const { return position; }

        /**
         * Sets the top left viewport position in pixels
         */
        inline auto setViewportSize(const vec2 size) { this->size = size; }

        /**
         * Sets the viewport size in pixels
         */
        inline auto setViewportPosition(const vec2 position) { this->position = position; }

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    private:
        vec2 position{};
        vec2 size{};
    };

}
