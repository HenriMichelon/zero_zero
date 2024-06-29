#pragma once

namespace z0 {

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

}