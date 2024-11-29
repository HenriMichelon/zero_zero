/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Rect;

export namespace z0 {

    namespace ui {
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
            inline bool contains(const float X, const float Y) const {
                return ((X >= x) && (X < (x + width)) &&
                        (Y >= y) && (Y < (y + height)));
            }

            /*! Returns true if the given rect is inside the rect */
            inline bool contains(const Rect& R) const {
                return ((R.x >= x) && (R.y >= y) &&
                        ((R.x + R.width) <= (x + width)) &&
                        ((R.y + R.height) <= (y + height)));
            }

            Rect& operator = (const Rect& R) = default;

            bool operator == (const Rect& R) const {
                return ((x == R.x) &&
                        (y == R.y) &&
                        (width == R.width) &&
                        (height == R.height));
            }

            /*! The rect is the result of the intersection between two rects */
            void intersect(const Rect& A, const Rect& B) {
                x = std::max(A.x, B.x);
                y = std::max(A.y, B.y);
                width = std::min(A.x + A.width, B.x + B.width) - x;
                height = std::min(A.y + A.height, B.y + B.height) - y;
            }
        };
    }

}