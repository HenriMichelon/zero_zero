#pragma once

#include "z0/color.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gresource_vector.h"

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
        void drawPanel(const GPanel&, GResourceVector&, VectorRenderer&) const;
        void drawBox(const GWidget&, GResourceVector&, VectorRenderer&) const;
        /*void drawLine(GLine&, GResourceVector&, VectorRenderer&);
        void drawFrame(GFrame&, GResourceVector&, VectorRenderer&);
        void drawArrow(GArrow&, GResourceVector&, VectorRenderer&);
        void drawButton(GButton&, GResourceVector&, VectorRenderer&);
        void drawCheckmark(GCheckmark&, GResourceVector&, VectorRenderer&);
        void drawText(GText&, GResourceVector&, VectorRenderer&);
        void drawTextEdit(GTextEdit&, GResourceVector&, VectorRenderer&);
        void drawProgressBar(GProgressBar&, GResourceVector&, VectorRenderer&);
        void drawSelection(GSelection&, GResourceVector&, VectorRenderer&);
        void drawTrackBar(GTrackBar&, GResourceVector&, VectorRenderer&);
        void drawToggleButton(GToggleButton&, GResourceVector&, VectorRenderer&);
        void drawRoundButton(GRoundButton&, GResourceVector&, VectorRenderer&);
        void drawTabButton(GTabButton&, GResourceVector&, VectorRenderer&);
        void drawTabs(GTabs&, GResourceVector&, VectorRenderer&);
        void drawGridCell(GGridCell&, GResourceVector&, VectorRenderer&);*/

    };

}