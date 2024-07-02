#pragma once

namespace z0 {

    class GScrollBar: public GValueSelect {
    public:
        enum Type {
            HORIZONTAL,
            VERTICAL
        };

        GScrollBar(Type = HORIZONTAL, 
                   float min = 0, 
                   float max = 100, 
                   float value = 0, 
                   float step = 1);
        
        Type getScrollBarType() const { return type; };

        void setResources(const string&, const string&);

    private:
        Type                type;
        bool		        onScroll{false};
        float	            scrollStart;
        shared_ptr<GBox>	liftArea;
        shared_ptr<GBox>	liftCage;

        bool eventMouseUp(MouseButton, float, float) override;
        bool eventMouseMove(uint32_t, float, float) override;
        void eventRangeChange() override;
        void eventValueChange(float) override;

        void onLiftAreaDown(GEventMouseButton*);
        void onLiftCageDown(GEventMouseButton*);

        void liftRefresh(const Rect &);

    };

    class GVScrollBar: public GScrollBar {
    public:
        GVScrollBar(uint32_t min = 0, uint32_t max = 100, uint32_t value = 0, uint32_t step = 1): 
        GScrollBar(VERTICAL, min, max, value, step) {};
    };


    class GHScrollBar: public GScrollBar {
    public:
        GHScrollBar(uint32_t min = 0, uint32_t max = 100, uint32_t value = 0, uint32_t step = 1): 
        GScrollBar(HORIZONTAL, min, max, value, step) {};
    };

}