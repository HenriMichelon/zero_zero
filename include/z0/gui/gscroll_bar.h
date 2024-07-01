#pragma once

namespace z0 {

    class GScrollBar: public GValueSelect {
    public:
        enum Type {
            HORIZONTAL,
            VERTICAL
        };

        GScrollBar(Type = HORIZONTAL, 
                   uint32_t min = 0, 
                   uint32_t max = 100, 
                   uint32_t value = 0, 
                   uint32_t step = 1,
                   const string& resArea = "", 
                   const string& resCage = "");
        
        Type getScrollBarType() const { return type; };

        void setResources(const string&, const string&);

    private:
        Type                type;
        bool		        onScroll{false};
        uint32_t	        scrollStart;
        shared_ptr<GBox>	liftArea;
        shared_ptr<GBox>	liftCage;


        bool eventMouseUp(MouseButton, float, float) override;
        bool eventMouseMove(uint32_t, float, float) override;
        void eventCreate() override;
        void eventResize() override;
        void eventRangeChange() override;
        void eventValueChange(float) override;

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