#pragma once

#include "z0/color.h"
#include "z0/gui/gstyle_classic_resource.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gline.h"
#include "z0/gui/gbutton.h"
#include "z0/gui/gtoggle_button.h"

namespace z0 {

    class Glayout;

    class GStyleClassic: public GStyle
    {
    public:
        GStyleClassic();
        ~GStyleClassic() override;

        bool init() override;
        void draw(const GWidget&, GResource&, VectorRenderer&, bool) const override;
        void addResource(GWidget&, const string&) override;
        void resize(GWidget&, Rect&, GResource&) override;

    private:
        Color	background;
        Color	focus;
        Color	shadowDark;
        Color	shadowBright;
        Color	fgUp;
        Color	fgDown;
        //GTexture	*texture;

        void updateOptions() override;
        Color extractColor(const string&, float, float, float);
        void drawPanel(const GPanel&, GStyleClassicResource&, VectorRenderer&) const;
        void drawBox(const GWidget&, GStyleClassicResource&, VectorRenderer&) const;
        void drawLine(GLine&, GStyleClassicResource&, VectorRenderer&) const;
        void drawButton(GButton&, GStyleClassicResource&, VectorRenderer&) const;
        void drawToggleButton(GToggleButton&, GStyleClassicResource&, VectorRenderer&) const;
        /*void drawFrame(GFrame&, GLayoutVectorResource&, VectorRenderer&);
        void drawArrow(GArrow&, GLayoutVectorResource&, VectorRenderer&);
        void drawCheckmark(GCheckmark&, GLayoutVectorResource&, VectorRenderer&);
        void drawText(GText&, GLayoutVectorResource&, VectorRenderer&);
        void drawTextEdit(GTextEdit&, GLayoutVectorResource&, VectorRenderer&);
        void drawProgressBar(GProgressBar&, GLayoutVectorResource&, VectorRenderer&);
        void drawSelection(GSelection&, GLayoutVectorResource&, VectorRenderer&);
        void drawTrackBar(GTrackBar&, GLayoutVectorResource&, VectorRenderer&);
        void drawRoundButton(GRoundButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawTabButton(GTabButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawTabs(GTabs&, GLayoutVectorResource&, VectorRenderer&);
        void drawGridCell(GGridCell&, GLayoutVectorResource&, VectorRenderer&);*/

    };

}