#pragma once

namespace z0 {

    /**
     * Two states clickeable button
     */
    class GToggleButton: public GCheckWidget {
    public:
        GToggleButton();
        virtual ~GToggleButton() {};

    protected:
        virtual bool eventMouseDown(MouseButton, float, float);
    };


}