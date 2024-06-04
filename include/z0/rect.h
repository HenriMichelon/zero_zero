#pragma once

namespace z0 {

    struct Rect {
        /*! Bottom-Left corner */
        float	x{0.0f};
        /*! Bottom-Left corner */
        float	y{0.0f};
        /*! Width */
        float width{0.0f};
        /*! Height  */
        float height{0.0f};

        /*! Return true if the given point is inside the rect */
        bool contains(float X, float Y) const;

        /*! Return true if the given rect is inside the rect */
        bool contains(const Rect&) const;

        /*! */
        Rect& operator = (const Rect&R);

        /*! */
        bool operator == (const Rect&R) const;

        /*! The rect is the resulst of the intersection between two rects */
        void intersect(const Rect&, const Rect&);
    };

}