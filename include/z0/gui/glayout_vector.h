#pragma once

#include "z0/color.h"
#include "z0/gui/glayout_vector_resource.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gline.h"

namespace z0 {

    class Glayout;

    class GLayoutVector: public GLayout
    {
    public:
        GLayoutVector();
        ~GLayoutVector() override;

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
        void drawPanel(const GPanel&, GLayoutVectorResource&, VectorRenderer&) const;
        void drawBox(const GWidget&, GLayoutVectorResource&, VectorRenderer&) const;
        void drawLine(GLine&, GLayoutVectorResource&, VectorRenderer&) const;
        /*void drawFrame(GFrame&, GLayoutVectorResource&, VectorRenderer&);
        void drawArrow(GArrow&, GLayoutVectorResource&, VectorRenderer&);
        void drawButton(GButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawCheckmark(GCheckmark&, GLayoutVectorResource&, VectorRenderer&);
        void drawText(GText&, GLayoutVectorResource&, VectorRenderer&);
        void drawTextEdit(GTextEdit&, GLayoutVectorResource&, VectorRenderer&);
        void drawProgressBar(GProgressBar&, GLayoutVectorResource&, VectorRenderer&);
        void drawSelection(GSelection&, GLayoutVectorResource&, VectorRenderer&);
        void drawTrackBar(GTrackBar&, GLayoutVectorResource&, VectorRenderer&);
        void drawToggleButton(GToggleButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawRoundButton(GRoundButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawTabButton(GTabButton&, GLayoutVectorResource&, VectorRenderer&);
        void drawTabs(GTabs&, GLayoutVectorResource&, VectorRenderer&);
        void drawGridCell(GGridCell&, GLayoutVectorResource&, VectorRenderer&);*/

    };

}