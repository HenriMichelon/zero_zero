#pragma once

namespace z0 {

    struct Rect {
        /*! Bottom-Left corner */
        int32_t	x{0};
        /*! Bottom-Left corner */
        int32_t	y{0};
        /*! Width */
        int32_t width{0};
        /*! Height  */
        int32_t height{0};

        /*! Return true if the given point is inside the rect */
        bool contains(int32_t X, int32_t Y) const;

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