#include "z0/z0.h"
#include "z0/rect.h"

namespace z0 {

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

    bool Rect :: contains(int32_t X, int32_t Y) const {
        return ((X >= x) && (X < (x + width)) &&
                (Y >= y) && (Y < (y + height)));
    }

    bool Rect :: contains(const Rect&R) const {
        return ((R.x >= x) && (R.y >= y) &&
                ((R.x + R.width) <= (x + width)) &&
                ((R.y + R.height) <= (y + height)));
    }



}