/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.StyleClassic;

import z0.ui.Image;

namespace z0::ui {

    bool StyleClassic::init() {
        setOption("color_focus", "");
        setOption("color_shadow_dark", "");
        setOption("color_shadow_light", "");
        setOption("color_foreground_up", "");
        setOption("color_foreground_down", "");
        setOption("color_background", "0.75,0.75,0.9");
        setOption("color_textcolor", "0.0,0.0,0.0");
        // setOption("texture", "vectorbg.png");
        return true;
    }

    void StyleClassic::draw(const Widget &widget, Resource &resources, VectorRenderer &renderer, const bool before) const {
        auto &res = static_cast<StyleClassicResource &>(resources);
        if (!widget.isVisible()) {
            return;
        }
        if (before) {
            switch (widget.getType()) {
                // case Widget::TEXTEDIT:
                // case Widget::UPDOWN:
                // case Widget::PROGRESSBAR:
            case Widget::PANEL:
                drawPanel((Panel &)widget, res, renderer);
                break;
            case Widget::BOX:
                // case Widget::SCROLLBOX:
                drawBox(widget, res, renderer);
                break;
            case Widget::LINE:
                drawLine((Line &)widget, res, renderer);
                break;
            case Widget::BUTTON:
                drawButton((Button &)widget, res, renderer);
                break;
            case Widget::TOGGLEBUTTON:
                drawToggleButton((ToggleButton &)(CheckWidget &)widget, res, renderer);
                break;
            case Widget::TEXT:
                drawText((Text &)widget, res, renderer);
                break;
            case Widget::FRAME:
                drawFrame((Frame &)widget, res, renderer);
                break;
            case Widget::IMAGE: {
                auto &pic = dynamic_cast<const Image &>(widget);
                if (pic.getImage()) {
                    renderer.setPenColor(pic.getColor());
                    renderer.drawFilledRect(widget.getRect(), widget.getRect().width, widget.getRect().height, pic.getImage());
                }
            }
                /*case Widget::GRIDCELL:
                    DrawGridCell((GGridCell&)W, D, res);
                    break;
                case Widget::ARROW:
                    DrawArrow((GArrow&)W, D, res);
                    break;
                case Widget::CHECKMARK:
                    DrawCheckmark((GCheckmark&)W, D, res);
                    break;
                case Widget::SELECTION:
                    DrawSelection((GSelection&)W, D, res);
                    break;
                case Widget::TRACKBAR:
                    DrawTrackBar((GTrackBar&)W, D, res);
                    break;
                case Widget::ROUNDBUTTON:
                    DrawRoundButton((GRoundButton&)W, D, res);
                    break;
                case Widget::TABBUTTON:
                    DrawTabButton((GTabButton&)W, D, res);
                    break;

                    break;*/
            default:
                break;
            }
        } else {
            switch (widget.getType()) {
                /*case Widget::TEXTEDIT:
                    DrawTextEdit((GTextEdit&)W, D, res, R);
                    break;
                case Widget::PROGRESSBAR:
                    DrawProgressBar((GProgressBar&)W, D, res, R);
                    break;
                case Widget::TABS:
                    DrawTabs((GTabs&)W, D, res, R);
                    break;*/
            default:
                break;
            }
        }
    }

    void StyleClassic::addResource(Widget &widget, const string &resources) {
        const auto& res = make_shared<StyleClassicResource>(resources);
        widget.setResource(res);
        widget._setSize(res->width, res->height);
        switch (widget.getType()) {
        case Widget::SCROLLBAR:
            static_cast<ScrollBar &>(widget).setResources(",,LOWERED", ",,RAISED");
            break;
        case Widget::TREEVIEW:
            static_cast<TreeView &>(widget).setResources(",,LOWERED", "18,18,RAISED", "");
            break;
            // XXX
            /*case Widget::UPDOWN:
                ((GUpDown&)W).SetResources(RES, RES);
                break;
            case Widget::TRACKBAR:
                ((GTrackBar&)W).SetResources("12,12,RAISED");
                break;
            case Widget::PROGRESSBAR:
                ((GProgressBar&)W).SetResources(RES);
                break;
            case Widget::LISTBOX:
                ((GListBox&)W).SetResources(string(",,LOWERED") + (res->flat ? ",FLAT" : ""),
                                            string("18,18,RAISED") + (res->flat ? ",FLAT" : ""),
                                            "");
                break;
            case Widget::SCROLLBOX:
                ((GScrollBox&)W).SetResources(string(",,LOWERED") + (res->flat ? ",FLAT" : ""),
                                              string("18,18,RAISED") + (res->flat ? ",FLAT" : ""),
                                              string("18,18,RAISED") + (res->flat ? ",FLAT" : ""));
                break;
            case Widget::TEXTEDIT:
                ((GTextEdit&)W).SetResources(RES);
                break;
            case Widget::TABS:
                ((GTabs&)W).SetResources(",,RAISED");
                break;
            case Widget::GRID:
                ((GGrid&)W).SetResources(",,LOWERED");*/
        default:
            break;
        }
    }

    void StyleClassic::resize(Widget &widget, Rect &rect, Resource &) {
        switch (widget.getType()) {
        case Widget::BOX:
        case Widget::BUTTON:
            // case Widget::TABBUTTON:
            widget.setVBorder(2);
            widget.setHBorder(2);
            break;
        case Widget::FRAME: {
            widget.setHBorder(4);
            float w, h;
            widget.getFont().getSize(static_cast<Frame &>(widget).getText(), w, h);
            widget.setVBorder(h - 2);
        } break;
            /*case Widget::LINE:
            {
                GLine &L = (GLine&)W;
                if (L.Style() == GLine::HORIZ) {
                    R.height = 2;
                }
                else if (L.Style() == GLine::VERT) {
                    R.width = 2;
                }
            }
                break;*/
        default:
            break;
        }
    }

    void StyleClassic::updateOptions() {
        focus        = extractColor("color_focus", 0.1f, 0.1f, 0.1f);
        shadowDark   = extractColor("color_shadow_dark", 0.3f, 0.3f, 0.3f);
        shadowBright = extractColor("color_shadow_light", 0.94f, 0.94f, 0.94f);
        fgUp         = extractColor("color_foreground_up", 0.68f, 0.68f, 0.81f, 0.25f);
        fgDown       = extractColor("color_foreground_down", 0.76f, 0.85f, 0.76f, 0.5f);
        background   = extractColor("color_background", 0.75f, 0.75f, 0.90f, 0.25f);
        /*XXXX
        if (texture != nullptr) { delete texture; }
        if (Option("texture").Len() > 0) {
            DatatypePixmap dtype;
            texture = new GTexture(dtype.Load(Option("texture")),
                                   background);
        }*/
    }

    vec4 StyleClassic::extractColor(const string &OPT, const float R, const float G, const float B, const float A) const {
        const string opt = getOption(OPT);
        if (!opt.empty()) {
            const auto &rgb = split(opt, ',');
            if (rgb.size() == 3) {
                return vec4({stof(string{rgb[0]}), stof(string{rgb[1]}), stof(string{rgb[2]}), 1.0f});
            }
            if (rgb.size() == 4) {
                return vec4{stof(string{rgb[0]}), stof(string{rgb[1]}), stof(string{rgb[2]}), stof(string{rgb[3]})};
            }
        }
        return vec4{R, G, B, A};
    }

    void StyleClassic::drawPanel(const Panel &widget, StyleClassicResource &, VectorRenderer &renderer) const {
        if (widget.isDrawBackground()) {
            auto c = background;
            c.a    = widget.getTransparency();
            renderer.setPenColor(c);
            renderer.drawFilledRect(widget.getRect(), widget.getRect().width, widget.getRect().height);
            // texture->Draw(D, W.Rect());
        }
    }

    void StyleClassic::drawBox(const Widget &widget, const StyleClassicResource &resources, VectorRenderer &renderer) const {
        if ((widget.getWidth() < 4) || (widget.getHeight() < 4)) {
            return;
        }
        float       l  = widget.getRect().x;
        float       b  = widget.getRect().y;
        const float w  = widget.getRect().width;
        const float h  = widget.getRect().height;
        auto        fd = fgDown;
        auto        fu = fgUp;
        fd.a  -= 1.0f-widget.getTransparency();
        fu.a  -= 1.0f-widget.getTransparency();
        if (widget.isDrawBackground()) {
            if (widget.isPushed()) {
                renderer.setPenColor(fd);
            } else {
                renderer.setPenColor(fu);
            }
            renderer.drawFilledRect(widget.getRect(), widget.getRect().width, widget.getRect().height);
        }
        if (resources.style != StyleClassicResource::FLAT) {
            auto sb = shadowBright;
            sb.a    = widget.getTransparency();
            auto sd = shadowDark;
            sd.a    = widget.getTransparency();
            switch (resources.style) {
            case StyleClassicResource::LOWERED:
                renderer.setPenColor(sb);
                break;
            case StyleClassicResource::RAISED:
                renderer.setPenColor(sd);
                break;
            default:
                break;
            }
            renderer.drawLine({l, b}, {l + w-1, b});
            renderer.drawLine({l, b}, {l, b + h-1});
            switch (resources.style) {
            case StyleClassicResource::RAISED:
                renderer.setPenColor(sb);
                break;
            case StyleClassicResource::LOWERED:
                renderer.setPenColor(sd);
                break;
            default:
                break;
            }
            renderer.drawLine({l + w-1, b}, {l + w-1, b + h-1});
            renderer.drawLine({l + w-1, b + h-1}, {l, b + h-1});
        }
    }

    void StyleClassic::drawLine(const Line &widget, const StyleClassicResource &resource, VectorRenderer &renderer) const {
        vec4 color;
        if (resource.customColor) {
            color = resource.color;
        } else {
            color = resource.style == StyleClassicResource::RAISED ? shadowDark : shadowBright;
        }
        color.a -= 1.0f - widget.getTransparency();
        auto& rect = widget.getRect();
        renderer.setPenColor(color);
        if (widget.getStyle() == Line::HORIZ) {
            renderer.drawLine({rect.x, rect.y}, {rect.x + rect.width, rect.y});
        } else if (widget.getStyle() == Line::VERT) {
            renderer.drawLine({rect.x, rect.y}, {rect.x, rect.y + rect.height});
        }
    }

    void StyleClassic::drawButton(const Button &widget, StyleClassicResource &resource, VectorRenderer &renderer) const {
        resource.style = widget.isPushed() ? StyleClassicResource::LOWERED : StyleClassicResource::RAISED;
        drawBox(widget, resource, renderer);
    }

    void StyleClassic::drawToggleButton(ToggleButton &widget, StyleClassicResource &resources, VectorRenderer &renderer) const {
        if (widget.getState() == CheckWidget::CHECK) {
            resources.style = StyleClassicResource::LOWERED;
            widget.setPushed(true);
        } else {
            resources.style = StyleClassicResource::RAISED;
            widget.setPushed(false);
        }
        drawBox(widget, resources, renderer);
    }

    void StyleClassic::drawText(Text &widget, StyleClassicResource &resources, VectorRenderer &renderer) const {
        renderer.setPenColor(vec4{widget.getTextColor().r, widget.getTextColor().g, widget.getTextColor().b, widget.getTransparency()});
        auto rect = widget.getRect();
        widget.getSize(rect.width, rect.height);
        renderer.drawText(widget.getText(), widget.getFont(), rect, widget.getRect().width, widget.getRect().height);
    }

    void StyleClassic::drawFrame(Frame &widget, StyleClassicResource &resources, VectorRenderer &renderer) const {
        if ((widget.getWidth() < 4) || (widget.getHeight() < 4)) {
            return;
        }
        constexpr int32_t LEFTOFFSET = 8;
        float             l          = widget.getRect().x;
        float             b          = widget.getRect().y;
        float             w          = widget.getRect().width;
        float             h          = widget.getRect().height;
        vec4              c1;
        vec4              c2;
        auto              sb = shadowBright;
        sb.a                 = widget.getTransparency();
        auto sd              = shadowDark;
        sd.a                 = widget.getTransparency();
        switch (resources.style) {
        case StyleClassicResource::LOWERED:
            c1 = sb;
            c2 = sd;
            break;
        case StyleClassicResource::RAISED:
            c1 = sd;
            c2 = sb;
            break;
        case StyleClassicResource::FLAT:
            c1 = sd;
            c2 = sd;
            break;
        }
        float fh, fw;
        widget.getFont().getSize(widget.getText(), fw, fh);
        const auto &ratio = app().getVectorRatio();
        fw                = roundf(fw / ratio.x);
        fh                = roundf(fh / ratio.y);
        renderer.setPenColor(c2);
        if ((!widget.getText().empty()) && (widget.getWidth() >= (fw + LEFTOFFSET)) && (widget.getHeight() >= fh)) {
            renderer.drawLine({l, b + h}, {l + LEFTOFFSET, b + h});
            renderer.drawLine({l + fw + LEFTOFFSET + 1, b + h}, {l + w, b + h});
            renderer.setPenColor(vec4{widget.getTextColor().r, widget.getTextColor().g, widget.getTextColor().b, widget.getTransparency()});
            renderer.drawText(widget.getText(), widget.getFont(), l + LEFTOFFSET, (b + h) - (fh / 2), fw, fh, fw, fh);
            renderer.setPenColor(c2);
        } else {
            renderer.drawLine({l + w, b + h}, {l, b + h});
        }
        renderer.drawLine({l + w, b}, {l + w, b + h});
        renderer.setPenColor(c1);
        renderer.drawLine({l, b}, {l + w, b});
        renderer.drawLine({l, b}, {l, b + h});
    }


    /*


    //----------------------------------------------
        void GLayoutVector::DrawArrow(GArrow&W, GLayoutVectorResource&, VectorRenderer&, float)
        {
            if ((W.Width() < 4) || (W.Height() < 4)) return;
            //dprintf("Draw Arrow %x\n", &W);

            uint32_t i, w, h, xpos, ypos, left, top;
            switch (W.Kind()) {
                case GArrow::UP:
                case GArrow::DOWN:
                    w = W.Width() - 4;
                    if (!(w % 2)) w--;
                    h = (w/2+1);
                    xpos = (W.Width() - w)/2;
                    ypos = (W.Width() - h)/2;
                    break;
                case GArrow::LEFT:
                case GArrow::RIGHT:
                default:
                    h = W.Height() - 4;
                    if (!(h % 2)) h--;
                    w = (h/2+1);
                    xpos = (W.Height() - w)/2;
                    ypos = (W.Height() - h)/2;
                    break;
            }


            left = W.Left() + xpos;
            top = W.Top() + ypos;
            if (W.Enabled())
                D.SetPenColor(shadowDark);
            else
                D.SetPenColor(fgUp);
            switch (W.Kind()) {
                case GArrow::UP:
                    for (i=0; i<h; i++)
                        D.DrawLine(i+left, h-i+top-1,
                                   (w-i)+left, h-i+top-1);
                    break;
                case GArrow::DOWN:
                    for (i=0; i<h; i++)
                        D.DrawLine(i+left, i+top,
                                   (w-i)+left, i+top);
                    break;
                case GArrow::LEFT:
                    for (i=0; i<w; i++)
                        D.DrawLine(w-i+left-1, top+i,
                                   w-i+left-1, top+h-i);
                    break;
                case GArrow::RIGHT:
                    for (i=0; i<w; i++)
                        D.DrawLine(i+left+1, top+i,
                                   i+left+1, top+h-i);
                    break;
            }
        }

    //----------------------------------------------
        void GLayoutVector::DrawCheckmark(GCheckmark&W, GLayoutVectorResource&, VectorRenderer&, float)
        {
            if ((W.Width() < 5) || (W.Height() < 5)) return;

            _LONG l = W.Left()+1;
            _LONG t = W.Top()+1;
            _LONG r = l+W.Width()-3;
            _LONG b = t+W.Height()-3;

            D.SetPenColor(shadowDark);
            //if (kind == CROSS)
            {
                D.DrawLine(l+1,	t,		r,		b-1);
                D.DrawLine(l,	t,		r,		b);
                D.DrawLine(l,	t+1,	r-1,	b);
                D.DrawLine(l,	b-1,	r-1,	t);
                D.DrawLine(l,	b,		r,		t);
                D.DrawLine(l+1,	b,		r,		t+1);
            }

        }

    //----------------------------------------------
        void GLayoutVector::DrawTextEdit(GTextEdit&W, GLayoutVectorResource&, VectorRenderer&, floatRESR)
        {
            if (W.HaveFocus() && (!W.ReadOnly())) {
                UStringz txt = W.DisplayedText().Left(W.SelStart() -
                                                      W.FirstDisplayedChar());
                uint32_t l = W.Left() + 2 + W.Font().Width(txt);
                uint32_t t = W.Top() + 2;
                uint32_t h = W.Font().Height();
                D.SetPenColor(shadowDark);
                D.DrawHLine(l - 2, t, 5);
                D.DrawHLine(l - 2, t + h, 5);
                D.DrawLine(l, t, l, t + h);
            }
        }



    //----------------------------------------------
        void GLayoutVector::DrawProgressBar(GProgressBar&W, GLayoutVectorResource&, VectorRenderer&, float)
        {
            D.SetPenColor(fgDown);
            if (W.Max() > W.Min()) {
                if (W.GProgressBar::Type() == GProgressBar::HORIZ)	{
                    D.DrawRect(W.Left() + 1,
                               W.Top() + 1,
                               (W.Value() - W.Min()) * (W.Width() - 1*2) /
                               ABS(W.Max() - W.Min()),
                               W.Height() - 1*2);
                    if (W.DisplayType() != GProgressBar::NONE) {
                        UStringz text;
                        uint32_t x = (W.Width() - 1*2 -
                                    W.Font().Width(text))/2;
                        _LONG y = (W.Height() - 1*2 -
                                   W.Font().Height())/2;
                        W.Font().Draw(D, text, x + W.Left() + 1, y + W.Top() + 1);
                    }
                }
                else
                {
                    uint32_t size = (W.Value()- W.Min()) * (W.Height()-1*2) /
                                  ABS(W.Max()-W.Min());
                    D.DrawRect(W.Left() + 1, W.Top() + W.Height() -
                                             size - 1, W.Width() - 1*2,
                               size);
                }
            }
        }


    //--------------------------------------------------------------------------
        void GLayoutVector::DrawSelection(GSelection&W, VectorRenderer&D,
                                          GLayoutVectorResource&)
        {
            D.SetPenColor(fgDown);
            D.DrawRect(W.Rect());
        }



    //--------------------------------------------------------------------------
        void GLayoutVector::DrawTrackBar(GTrackBar&W, VectorRenderer&D,
                                         GLayoutVectorResource&)
        {
            if (W.TrackBarType() == GTrackBar::HORIZ) {
                _LONG left = W.Left() + W.tracker.Width()/2 - 1;
                _LONG right = W.Left() + W.Width() - W.tracker.Width()/2 - 1;
                D.SetPenColor(shadowBright);
                D.DrawHLine(left, W.Top() + W.Height() / 2, right - left - 1);
                D.SetPenColor(shadowDark);
                D.DrawLine(left, W.Top() + 2, left, W.Top() + W.Height() - 2);
                D.DrawLine(right, W.Top() + 2, right, W.Top() + W.Height() - 2);
                for (_LONG idx = W.Min(); idx < W.Max(); idx += W.Step()) {
                    _LONG l = left + (idx - W.Min()) *
                                     (W.Width() - W.tracker.Width()) / (W.Max() - W.Min());
                    D.DrawLine(l, W.Top() + W.Height()/4, l,
                               W.Top() + W.Height() - W.Height()/4);
                }
            }
            else {
                _LONG top = W.Top() + W.tracker.Height()/2 - 1;
                _LONG bottom = W.Top() + W.Height() - W.tracker.Height()/2 - 1;
                D.SetPenColor(shadowBright);
                D.DrawLine(W.Left() + W.Width() / 2, top, W.Left() + W.Width() / 2, bottom);
                D.SetPenColor(shadowDark);
                D.DrawLine(W.Left() + 2, top, W.Left() + W.Width() - 2, top);
                D.DrawLine(W.Left() + 2, bottom, W.Left() + W.Width() - 2, bottom);
                for (_LONG idx = W.Min(); idx < W.Max(); idx += W.Step()) {
                    _LONG t = top + (idx - W.Min()) *
                                    (W.Height() - W.tracker.Height()) / (W.Max() - W.Min());
                    D.DrawLine(W.Left() + W.Width()/4, t,
                               W.Left() + W.Width() - W.Width()/4, t);
                }
            }
        }


    //--------------------------------------------------------------------------
        void GLayoutVector::DrawTabs(GTabs&W, VectorRenderer&D,
                                     GLayoutVectorResource&)
        {
            D.SetPenColor(shadowBright);
            if (W.Childs().Count() == 0) {
                D.DrawLine(W.Left(), W.Top() + W.Height() - 1,
                           W.Left() + W.Width() - 1,  W.Top() + W.Height() - 1);
                return;
            }
            uint32_t wtab = MAX(_LONG(W.Width() / (W.Childs().Count() ) - W.Padding() * 2),
                              0l);
            uint32_t nsel = W.IndexSelected() - 1;
            uint32_t t = W.Top() + W.Height() - 1;
            D.DrawLine(W.Left(), t,
                       W.Left() + wtab * nsel - 1, t);
            D.SetPenColor(background);
            D.DrawLine(W.Left() + wtab * nsel, t,
                       W.Left() + wtab * (nsel + 1) - 2, t);
            D.SetPenColor(shadowBright);
            D.DrawLine(W.Left() + wtab * (nsel + 1) - 1, t,
                       W.Left() + W.Width() - 1, t);
        }


    //--------------------------------------------------------------------------
        void GLayoutVector::DrawTabButton(GTabButton&W, VectorRenderer&D,
                                          GLayoutVectorResource&RES)
        {
            if ((W.Width()<4) || (W.Height()<4)) { return; }
            uint32_t l = W.Rect().left;
            uint32_t t = W.Rect().top;
            uint32_t w = W.Rect().width;
            uint32_t h = W.Rect().height;
            w -= 1;
            h -= 2;
            D.SetPenColor(shadowBright);
            D.DrawLine(l, t, l+w, t);
            D.DrawLine(l, t, l, t+h);
            D.SetPenColor(shadowDark);
            D.DrawLine(l+w, t, l+w, t+h);
            l += 1;
            t += 1;
            w -= 1;
            if (W.DrawBackground() && (!RES.flat)) {
                if (W.Pushed() && W.MoveChildsOnPush()) {
                    D.SetPenColor(fgDown);
                    D.DrawRect(l, t, w, h);
                }
                else if (W.State() == GCheckWidget::CHECK) {
                    //D.SetPenColor(background);
                    texture->Draw(D, l, t, w, h+1);
                }
                else {
                    D.SetPenColor(fgUp);
                    D.DrawRect(l, t, w, h);
                }
            }
        }


    //--------------------------------------------------------------------------
        void GLayoutVector::DrawRoundButton(GRoundButton&W, VectorRenderer&D,
                                            GLayoutVectorResource&)
        {
            uint32_t beam = MIN(W.Width() / 2, W.Height() / 2);
            uint32_t outer = beam / 3;
            beam -= outer;
            _LONG x = W.Left() + W.Width() / 2;
            _LONG y = W.Top() + W.Height() / 2;

            const _DOUBLE RAD = 1.57079632679;
            //const uint32_t OFFSET = 3;

            _DOUBLE aMin;
            switch (W.RoundButtonType()) {
                case GRoundButton::RB_TOP: aMin = RAD; break;
                case GRoundButton::RB_LEFT: aMin = RAD * 2; break;
                case GRoundButton::RB_BOTTOM: aMin = RAD * 3; break;
                case GRoundButton::RB_RIGHT:
                default: aMin = 0.0; break;
            }

            _DOUBLE angle = ((RAD * 4 * (W.Value() - W.Min())) / (W.Max() - W.Min())) + aMin;

            D.SetPenColor(shadowBright);
            D.DrawCircle(x - 1,
                         y - 1,
                         beam,
                         FALSE);
            D.SetPenColor(shadowDark);
            D.DrawCircle(x + 1,
                         y + 1,
                         beam,
                         FALSE);
            //D.SetPenColor(fgUp);
            if (W.Pushed()) {
                D.SetPenColor(fgDown);
            }
            else {
                D.SetPenColor(fgUp);
            }
            D.DrawCircle(x, y, beam);

            D.SetPenColor(shadowBright);
            D.DrawCircle(x, y, beam + outer / 2 + 2, FALSE, W.Step());

            D.SetPenColor(shadowDark);
            D.DrawCircle(x, y, beam + outer / 2 + 1, FALSE, W.Step());
            D.DrawLine(x + _LONG(beam / 3 * cos(angle)),
                       y - _LONG(beam / 3 * sin(angle)),
                       x + _LONG((beam )  * cos(angle)),
                       y - _LONG((beam )* sin(angle)));
            D.DrawLine(x + _LONG((beam + outer) * cos(aMin)),
                       y - _LONG((beam + outer) * sin(aMin)),
                       x + _LONG((beam + 2) * cos(aMin)),
                       y - _LONG((beam + 2) * sin(aMin)));
        }


    //--------------------------------------------------------------------------
        void GLayoutVector::DrawGridCell(GGridCell&W, VectorRenderer&D,
                                         GLayoutVectorResource&RES)
        {
            if ((W.Width()<2) || (W.Height()<2)) { return; }
            uint32_t l = W.Rect().left;
            uint32_t t = W.Rect().top;
            uint32_t w = W.Rect().width;
            uint32_t h = W.Rect().height;
            --w;
            --h;
            switch (RES.style) {
                case GLayoutVectorResource::LOWERED:
                    D.SetPenColor(shadowBright);
                    break;
                default:
                    D.SetPenColor(shadowDark);
                    break;
            }
            D.DrawLine(l+w, t, l+w, t+h);
            D.DrawLine(l+w, t+h, l, t+h);

            if (W.DrawBackground()) {
                if (W.Pushed() || W.Selected()) {
                    D.SetPenColor(fgDown);
                }
                else {
                    D.SetPenColor(fgUp);
                }
                D.DrawRect(l, t, w, h);
            }
        }*/
} // namespace z0::ui
