#pragma once

#include <cstdint>

namespace z0 {

    struct GRect {
        /*! Top-Left corner (in pixels from the Top-Left of the screen)*/
        int32_t	left;
        /*! Top-Left corner (in pixels from the Top-Left of the screen)*/
        int32_t	top;
        /*! Width in pixels */
        uint32_t	width;
        /*! Height in pixels */
        uint32_t	height;

        /*! Create a rectangle with given size & position
            \param int32_t:	Left coord
            \param int32_t:	Top coord
            \param uint32_t:	width
            \param uint32_t:	height
        */
        explicit GRect(int32_t x = 0, int32_t y = 0, uint32_t w = 0, uint32_t h = 0):
        left(x), top(y), width(w), height(h) {};

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