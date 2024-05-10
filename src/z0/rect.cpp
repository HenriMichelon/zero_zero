#include <algorithm>
#include "z0/rect.h"

#include <algorithm>
using namespace std;

namespace z0 {

    void Rect ::intersect(const Rect&A, const Rect&B) {
        x = max(A.x, B.x);
        y = max(A.y, B.y);
        width = min(A.x + A.width, B.x + B.width) - x;
        height = min(A.y + A.height, B.y + B.height) - y;
    }

    Rect& Rect :: operator = (const Rect&R) {
        x = R.x;
        y = R.y;
        width = R.width;
        height = R.height;
        return *this;
    }

    bool Rect :: operator == (const Rect&R) const {
        return ((x == R.x) &&
                (y == R.y) &&
                (width == R.width) &&
                (height == R.height));
    }

    bool Rect :: contains(uint32_t X, uint32_t Y) const {
        return ((X >= x) && (X < int32_t((x + width))) &&
                (Y >= y) && (Y < int32_t((y + height))));
    }

    bool Rect :: contains(const Rect&R) const {
        return ((R.x >= x) && (R.y >= y) &&
                ((R.x + R.width) <= (x + width)) &&
                ((R.y + R.height) <= (y + height)));
    }



}