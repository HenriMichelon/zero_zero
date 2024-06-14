#pragma once

namespace z0 {

    /**
     * Define a rectangular area with a bottom-left origin
     */
    struct Rect {
        /*! Bottom-Left corner X position*/
        float	x{0.0f};
        /*! Bottom-Left corner Y position*/
        float	y{0.0f};
        /*! Width */
        float width{0.0f};
        /*! Height  */
        float height{0.0f};

        /*! Returns true if the given point is inside the rect */
        bool contains(float x, float y) const;

        /*! Returns true if the given rect is inside the rect */
        bool contains(const Rect&) const;

        Rect& operator = (const Rect&R);

        bool operator == (const Rect&R) const;

        /*! The rect is the resulst of the intersection between two rects */
        void intersect(const Rect&, const Rect&);
    };

}