#pragma once

namespace z0 {

    // Two states button
    class GToggleButton: public GCheckWidget {
    public:
        GToggleButton();
        virtual ~GToggleButton() {};

    protected:
        virtual bool eventMouseDown(MouseButton, uint32_t, uint32_t);
    };


}