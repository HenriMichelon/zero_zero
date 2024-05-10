#pragma once

#include <cstdint>

namespace z0 {

    struct Rect {
        /*! Bottom-Left corner */
        int32_t	x;
        /*! Bottom-Left corner */
        int32_t	y;
        /*! Width */
        uint32_t width;
        /*! Height  */
        uint32_t height;

        /*! Return true if the given point is inside the rect */
        bool contains(uint32_t X, uint32_t Y) const;

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