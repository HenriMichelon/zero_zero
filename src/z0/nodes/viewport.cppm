module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

export module z0:Viewport;

import :Node;

export namespace z0 {

    /**
     * Node that define the scene renderer viewport position & size
     */
    class Viewport : public Node {
    public:
        /**
         * Creates a Viewport
         * @param position top left viewport position in pixels
         * @param size size in pixels
         * @param name node name
         */
        Viewport(const vec2 &position, const vec2 &size, const string &name = "Viewport"):
            Node{name, VIEWPORT}, position{position}, size{size} {
        }

        explicit Viewport(const string &name = "Viewport"):
            Node{name, VIEWPORT} {
        }

        /**
         * Sets the viewport size in pixels
         */
        [[nodiscard]] inline const vec2 &getViewportSize() const { return size; }

        /**
         * Sets the top left viewport position in pixels
         */
        [[nodiscard]] inline const vec2 &getViewportPosition() const { return position; }

        /**
         * Sets the top left viewport position in pixels
         */
        void setViewportSize(const vec2 size) {
            this->size = size;
        }

        /**
         * Sets the viewport size in pixels
         */
        void setViewportPosition(const vec2 position) {
            this->position = position;
        }

    private:
        vec2 position{};
        vec2 size{};
    };

}
