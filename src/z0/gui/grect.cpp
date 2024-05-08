#include <algorithm>
#include "z0/gui/grect.h"

#include <algorithm>
using namespace std;

namespace z0 {

//----------------------------------------------------------------------------
    void GRect ::intersect(const GRect&A, const GRect&B)
    {
        left = max(A.left, B.left);
        top = max(A.top, B.top);
        width = min(A.left + A.width, B.left + B.width) - left;
        height = min(A.top + A.height, B.top + B.height) - top;
    }


//----------------------------------------------------------------------------
    GRect& GRect :: operator = (const GRect&R)
    {
        left = R.left;
        top = R.top;
        width = R.width;
        height = R.height;
        return *this;
    }


//----------------------------------------------------------------------------
    bool GRect :: operator == (const GRect&R) const
    {
        return ((left == R.left) &&
                (top == R.top) &&
                (width == R.width) &&
                (height == R.height));
    }


//----------------------------------------------------------------------------
    bool GRect :: contains(uint32_t X, uint32_t Y) const
    {
        return ((X >= left) && (X < int32_t((left + width))) &&
                (Y >= top) && (Y < int32_t((top + height))));
    }


//----------------------------------------------------------------------------
    bool GRect :: contains(const GRect&R) const
    {
        return ((R.left >= left) && (R.top >= top) &&
                ((R.left + R.width) <= (left + width)) &&
                ((R.top + R.height) <= (top + height)));
    }



}