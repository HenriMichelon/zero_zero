#pragma once

namespace z0 {

    class GScrollBar: public GValueSelect {
    public:
        enum Type {
            HORIZONTAL,
            VERTICAL
        };

        GScrollBar(Type = HORIZONTAL, uint32_t = 0, uint32_t = 0, uint32_t = 0, uint32_t = 1);
        
        Type getScrollBarType() const { return type; };

        void setResources(const string&, const string&);

    private:
        Type                type;
        bool		        onScroll;
        uint32_t	        scrollStart;
        shared_ptr<GBox>	liftArea;
        shared_ptr<GBox>	liftCage;


        virtual bool eventMouseUp(MouseButton, float, float);
        virtual bool eventMouseMove(MouseButton, float, float);
        virtual void eventRangeChange();
        virtual void eventValueChange(float);

        void onLiftAreaDown(GEventMouseButton*);
        void onLiftCageDown(GEventMouseButton*);

    };

    class GVScrollBar: public GScrollBar {
    public:
        GVScrollBar(uint32_t I = 0, uint32_t A = 0, uint32_t V = 0, uint32_t S = 1): 
        GScrollBar(VERTICAL, I, A, V, S) {};
    };


    class GHScrollBar: public GScrollBar {
    public:
        GHScrollBar(uint32_t I = 0, uint32_t A = 0, uint32_t V = 0, uint32_t S = 1): 
        GScrollBar(HORIZONTAL, I, A, V, S) {};
    };

}