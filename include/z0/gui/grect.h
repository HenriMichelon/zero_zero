#pragma once

#include <cstdint>

namespace z0 {

    struct GRect {
        /*! Top-Left corner */
        int32_t	left;
        /*! Top-Left corner */
        int32_t	top;
        /*! Width */
        uint32_t width;
        /*! Height  */
        uint32_t height;

        /*! Return true if the given point is inside the rect */
        bool contains(uint32_t X, uint32_t Y) const;

        /*! Return true if the given rect is inside the rect */
        bool contains(const GRect&) const;

        /*! */
        GRect& operator = (const GRect&R);

        /*! */
        bool operator == (const GRect&R) const;

        /*! The rect is the resulst of the intersection between two rects */
        void intersect(const GRect&, const GRect&);
    };

}