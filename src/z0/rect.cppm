module;
#include "z0/libraries.h"

export module Z0:Rect;

export namespace z0 {

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


    void Rect ::intersect(const Rect&A, const Rect&B) {
        x = std::max(A.x, B.x);
        y = std::max(A.y, B.y);
        width = std::min(A.x + A.width, B.x + B.width) - x;
        height = std::min(A.y + A.height, B.y + B.height) - y;
    }

    Rect& Rect :: operator = (const Rect&R) = default;

    bool Rect :: operator == (const Rect&R) const {
        return ((x == R.x) &&
                (y == R.y) &&
                (width == R.width) &&
                (height == R.height));
    }

    bool Rect :: contains(float X, float Y) const {
        return ((X >= x) && (X < (x + width)) &&
                (Y >= y) && (Y < (y + height)));
    }

    bool Rect :: contains(const Rect&R) const {
        return ((R.x >= x) && (R.y >= y) &&
                ((R.x + R.width) <= (x + width)) &&
                ((R.y + R.height) <= (y + height)));
    }


}