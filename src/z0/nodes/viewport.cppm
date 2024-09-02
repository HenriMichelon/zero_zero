module;
#include "z0/jolt.h"
#include "z0/modules.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

export module Z0:Viewport;

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
         */
        Viewport(vec2 position, vec2 size, const string& name = "Viewport");


        /**
         * Sets the viewport size in pixels
         */
        [[nodiscard]] inline const vec2& getViewportSize() const { return size; }

        /**
         * Sets the top left viewport position in pixels
         */
        [[nodiscard]] inline const vec2& getViewportPosition() const { return position; }

        /**
         * Sets the top left viewport position in pixels
         */
        void setViewportSize(vec2 size);

        /**
         * Sets the viewport size in pixels
         */
        void setViewportPosition(vec2 position);

    private:
        vec2 position;
        vec2 size;
    };

     Viewport::Viewport(vec2 _pos, vec2 _size, const string& name):
        Node{name}, position{_pos}, size{_size} {
    }

    void Viewport::setViewportSize(vec2 _size) {
        size = _size;
    }

    void Viewport::setViewportPosition(vec2 _position) {
        position = _position;
    }

}