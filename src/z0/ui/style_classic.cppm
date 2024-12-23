/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.StyleClassic;

import z0.Tools;
import z0.Application;

import z0.ui.Button;
import z0.ui.CheckWidget;
import z0.ui.Frame;
import z0.ui.Line;
import z0.ui.Panel;
import z0.ui.Rect;
import z0.ui.Resource;
import z0.ui.ScrollBar;
import z0.ui.Style;
import z0.ui.StyleClassicResource;
import z0.ui.Text;
import z0.ui.ToggleButton;
import z0.ui.TreeView;
import z0.ui.Widget;

import z0.vulkan.VectorRenderer;

export namespace z0 {

    namespace ui {

        class Glayout;

        class StyleClassic : public Style {
        public:
            ~StyleClassic() override = default;

            bool init() override;

            void draw(const Widget &W, Resource &RES, VectorRenderer &D, bool BEFORE) const override;

            void addResource(Widget &W, const string &RES) override;

            void resize(Widget &W, Rect &R, Resource &) override;

        private:
            vec4 background{};
            vec4 focus{};
            vec4 shadowDark{};
            vec4 shadowBright{};
            vec4 fgUp{};
            vec4 fgDown{};
            //GTexture	*texture;

            void updateOptions() override;

            vec4 extractColor(const string &OPT, const float R, const float G, const float B);

            void drawPanel(const Panel &, StyleClassicResource &, VectorRenderer &) const;

            void drawBox(const Widget &, StyleClassicResource &, VectorRenderer &) const;

            void drawLine(Line &, StyleClassicResource &, VectorRenderer &) const;

            void drawButton(Button &, StyleClassicResource &, VectorRenderer &) const;

            void drawToggleButton(ToggleButton &, StyleClassicResource &, VectorRenderer &) const;

            void drawText(Text &, StyleClassicResource &, VectorRenderer &) const;

            void drawFrame(Frame &, StyleClassicResource &, VectorRenderer &) const;

            /*void drawArrow(GArrow&, GLayoutVectorResource&, VectorRenderer&);
            void drawCheckmark(GCheckmark&, GLayoutVectorResource&, VectorRenderer&);
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
}
