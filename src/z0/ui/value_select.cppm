/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>

export module z0.ui.ValueSelect;

import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        class ValueSelect : public Widget {
        public:
            ValueSelect(Type T,
                         float MIN,
                         float MAX,
                         float VAL,
                         float STEP);

            ~ValueSelect() override = default;

            float getMin() const { return min; }
            float getMax() const { return max; }
            float getValue() const { return value; }
            float getStep() const { return step; }

            virtual void setMin(float V);

            virtual void setMax(float V);

            virtual void setValue(float V);

            virtual void setStep(float V);

        protected:
            float min;
            float max;
            float value;
            float step;

            void eventResize() override;

            virtual void eventRangeChange();

            virtual void eventValueChange(float prev);
        };
    }
}
