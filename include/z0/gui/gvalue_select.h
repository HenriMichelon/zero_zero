#pragma once

namespace z0 {

    class GValueSelect: public GPanel
    {
    public:
        GValueSelect(Type T);
        virtual ~GValueSelect() {};
        
        float getMin() const;
        float getMax() const;
        float getValue() const;
        float getStep() const;
        
        virtual void setMin(float);
        virtual void setMax(float);
        virtual void setValue(float);
        virtual void setStep(float);

    protected:
        float min;
        float max;
        float value;
        float step;

        virtual void eventResize();
        virtual void eventRangeChange();
        virtual void eventValueChange(float);
    };

}