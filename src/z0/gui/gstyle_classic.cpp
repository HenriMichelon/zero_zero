#include "z0/gui/gstyle_classic.h"
#include "z0/application.h"

namespace z0 {

    GStyleClassic::GStyleClassic(): GStyle()//, texture(nullptr)
    {
    }
    
    GStyleClassic::~GStyleClassic() {
        //if (texture) { delete texture; }
    }

    bool GStyleClassic::init() {
        setOption("color_focus", "");
        setOption("color_shadow_dark", "");
        setOption("color_shadow_light", "");
        setOption("color_foreground_up", "");
        setOption("color_foreground_down", "");
        setOption("color_background", "0.75,0.75,0.9");
        setOption("color_textcolor", "0.0,0.0,0.0");
        //setOption("texture", "vectorbg.png");
        return true;
    }

    Color GStyleClassic::extractColor(const string&OPT, float R, float G, float B) {
        string opt = getOption(OPT);
        if (!opt.empty()) {
            auto rgb = split(opt, ',');
            if (rgb.size() == 3) {
                return Color(vec3{stof(string{rgb[0]}),stof(string{rgb[1]}),stof(string{rgb[2]})});
            } else if (rgb.size() == 4) {
                return Color(vec4{stof(string{rgb[0]}),stof(string{rgb[1]}),stof(string{rgb[2]}),stof(string{rgb[3]})});
            }
        }
        return Color(vec3{R, G, B});
    }

    void GStyleClassic::updateOptions() {
        focus = extractColor("color_focus", 0.1, 0.1, 0.1);
        shadowDark = extractColor("color_shadow_dark", 0.3, 0.3, 0.3);
        shadowBright = extractColor("color_shadow_light", 0.94, 0.94, 0.94);
        fgUp = extractColor("color_foreground_up", 0.68, 0.68, 0.81);
        fgDown = extractColor("color_foreground_down", 0.76, 0.85, 0.76);
        background = extractColor("color_background", 0.75, 0.75, 0.90);
        text = extractColor("color_textcolor", 0.0, 0.0, 0.0);
        /*XXXX
        if (texture != nullptr) { delete texture; }
        if (Option("texture").Len() > 0) {
            DatatypePixmap dtype;
            texture = new GTexture(dtype.Load(Option("texture")),
                                   background);
        }*/
    }

    void GStyleClassic::addResource(GWidget&W, const string&RES)
    {
        auto res = make_shared<GStyleClassicResource>(RES);
        W.setResource(res);
        W.setSize(res->width, res->height);
        switch (W.getType()) {
            case GWidget::BOX:
                if (res->flat) { W.setTransparent(true); }
                break;
            // XXX
            /*case GWidget::UPDOWN:
                ((GUpDown&)W).SetResources(RES, RES);
                break;
            case GWidget::TRACKBAR:
                ((GTrackBar&)W).SetResources("12,12,RAISED");
                break;
            case GWidget::SCROLLBAR:
                ((GScrollBar&)W).SetResources(RES, RES,
                                              string(",,LOWERED") + (res->flat ? ",FLAT" : ""),
                                              string(",,RAISED") + (res->flat ? ",FLAT": ""));
                break;
            case GWidget::PROGRESSBAR:
                ((GProgressBar&)W).SetResources(RES);
                break;
            case GWidget::LISTBOX:
                ((GListBox&)W).SetResources(string(",,LOWERED") + (res->flat ? ",FLAT" : ""),
                                            string("18,18,RAISED") + (res->flat ? ",FLAT" : ""),
                                            "");
                break;
            case GWidget::SCROLLBOX:
                ((GScrollBox&)W).SetResources(string(",,LOWERED") + (res->flat ? ",FLAT" : ""),
                                              string("18,18,RAISED") + (res->flat ? ",FLAT" : ""),
                                              string("18,18,RAISED") + (res->flat ? ",FLAT" : ""));
                break;
            case GWidget::TEXTEDIT:
                ((GTextEdit&)W).SetResources(RES);
                break;
            case GWidget::TABS:
                ((GTabs&)W).SetResources(",,RAISED");
                break;
            case GWidget::GRID:
                ((GGrid&)W).SetResources(",,LOWERED");*/
            default:
                break;
        }
    }

    void GStyleClassic::resize(GWidget&W, Rect&R, GResource&) {
        switch (W.getType()) {
            case GWidget::BOX:
            case GWidget::BUTTON:
            case GWidget::TABBUTTON:
                W.setVBorder(2);
                W.setHBorder(2);
                break;
            /*case GWidget::LINE:
            {
                GLine &L = (GLine&)W;
                if (L.Style() == GLine::HORIZ) {
                    R.height = 2;
                }
                else if (L.Style() == GLine::VERT) {
                    R.width = 2;
                }
            }
                break;
            case GWidget::FRAME:
                W.SetHBorder(4);
                W.SetVBorder(W.Font().Height()+2);
                break;*/
            default:
                break;
        }
    }

    void GStyleClassic::draw(const GWidget&W, GResource&RES, VectorRenderer&D, bool BEFORE) const {
        auto &res = (GStyleClassicResource &)RES;
        if (!W.isVisible()) { return; }
        if (BEFORE) {
            switch (W.getType()) {
                //case GWidget::TEXTEDIT: 
                //case GWidget::UPDOWN:
                //case GWidget::SCROLLBAR:
                //case GWidget::PROGRESSBAR:
                case GWidget::PANEL:
                    drawPanel((GPanel&)W, res, D);
                    break;
                case GWidget::BOX:
                case GWidget::SCROLLBOX:
                    drawBox(W, res, D);
                    break;
                case GWidget::LINE:
                    drawLine((GLine&)W, res, D);
                    break;
                case GWidget::BUTTON:
                    drawButton((GButton&)W, res, D);
                    break;
                case GWidget::TOGGLEBUTTON:
                    drawToggleButton((GToggleButton&)(GCheckWidget&)W, res, D);
                    break;
                case GWidget::TEXT:
                    drawText((GText&)W, res, D);
                    break;
                /*case GWidget::GRIDCELL:
                    DrawGridCell((GGridCell&)W, D, res);
                    break;
                case GWidget::FRAME:
                    DrawFrame((GFrame&)W, D, res);
                    break;
                case GWidget::ARROW:
                    DrawArrow((GArrow&)W, D, res);
                    break;
                case GWidget::CHECKMARK:
                    DrawCheckmark((GCheckmark&)W, D, res);
                    break;
                case GWidget::SELECTION:
                    DrawSelection((GSelection&)W, D, res);
                    break;
                case GWidget::TRACKBAR:
                    DrawTrackBar((GTrackBar&)W, D, res);
                    break;
                case GWidget::ROUNDBUTTON:
                    DrawRoundButton((GRoundButton&)W, D, res);
                    break;
                case GWidget::TABBUTTON:
                    DrawTabButton((GTabButton&)W, D, res);
                    break;
                case GWidget::PICTURE:
                {
                    GPicture &pic = (GPicture&)W;
                    if (pic.Pixmap()) { pic.Pixmap()->Draw(D, pic.Left(), pic.Top(), pic.Transparency(), true); }
                }
                    break;*/
                default:
                    break;
            }
        }
        else {
            switch (W.getType()) {
                /*case GWidget::TEXTEDIT:
                    DrawTextEdit((GTextEdit&)W, D, res, R);
                    break;
                case GWidget::PROGRESSBAR:
                    DrawProgressBar((GProgressBar&)W, D, res, R);
                    break;
                case GWidget::TABS:
                    DrawTabs((GTabs&)W, D, res, R);
                    break;*/
                default:
                    break;
            }
        }
    }

    void GStyleClassic::drawPanel(const GPanel&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        if (W.isDrawBackground()) {
            D.setPenColor(background);
            D.drawFilledRect(W.getRect());
            //texture->Draw(D, W.Rect());
        }
    }

    void GStyleClassic::drawBox(const GWidget&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        if ((W.getWidth()<4) || (W.getHeight()<4)) { return; }
        uint32_t l = W.getRect().x;
        uint32_t b = W.getRect().y;
        uint32_t w = W.getRect().width;
        uint32_t h = W.getRect().height;
        if (W.isDrawBackground())	{
            if (W.isPushed()) {
                D.setPenColor(fgDown);
            }
            else {
                D.setPenColor(fgUp);
            }
            D.drawFilledRect(W.getRect());
        }
        if (RES.style != GStyleClassicResource::FLAT) {
            switch (RES.style) {
                case GStyleClassicResource::LOWERED:
                    D.setPenColor(shadowBright);
                    break;
                case GStyleClassicResource::RAISED:
                    D.setPenColor(shadowDark);
                    break;
                default:
                    break;
            }
            D.drawLine({l, b}, {l+w, b});
            D.drawLine({l, b}, {l, b+h});
            switch (RES.style) {
                case GStyleClassicResource::RAISED:
                    D.setPenColor(shadowBright);
                    break;
                case GStyleClassicResource::LOWERED:
                    D.setPenColor(shadowDark);
                    break;
                default:
                    break;
            }
            D.drawLine({l+w, b}, {l+w, b+h});
            D.drawLine({l+w, b+h}, {l, b+h});

            l += 1;
            b += 1;
            w -= 1;
            h -= 1;
        }
    }

    void GStyleClassic::drawLine(GLine&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        Color c1, c2;
        switch (RES.style) {
            case GStyleClassicResource::RAISED:
                c1 = shadowDark;
                c2 = shadowBright;
                break;
            case GStyleClassicResource::LOWERED:
                c1 = shadowBright;
                c2 = shadowDark;
                break;
            default:
                c1 = shadowDark;
                c2 = shadowDark;
                break;
        }
        auto rect = W.getRect();
        D.setPenColor(c1);
        if (W.getStyle() == GLine::HORIZ)
            D.drawLine({rect.x, rect.y}, {rect.x+rect.width, rect.y});
        else if (W.getStyle() == GLine::VERT)
            D.drawLine({rect.x, rect.y}, {rect.x, rect.y+rect.height});
        D.setPenColor(c2);
        if (W.getStyle() == GLine::HORIZ)
            D.drawLine({rect.x, rect.y+1}, {rect.x+rect.width, rect.y+1});
        else if (W.getStyle() == GLine::VERT)
            D.drawLine({rect.x+1, rect.y}, {rect.x+1, rect.y+rect.height});
    }

    void GStyleClassic::drawButton(GButton&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        if (W.isPushed()) {
            RES.style = GStyleClassicResource::LOWERED;
        }
        else {
            if (RES.flat) {
                if (W.isPointed()) {
                    RES.style = GStyleClassicResource::RAISED;
                }
                else {
                    RES.style = GStyleClassicResource::FLAT;
                }
            }
            else {
                RES.style = GStyleClassicResource::RAISED;
            }
        }
        drawBox(W, RES, D);
    }

    void GStyleClassic::drawToggleButton(GToggleButton&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        W.setDrawBackground(!RES.flat);
        if (W.getState() == GCheckWidget::CHECK) {
            RES.style = GStyleClassicResource::LOWERED;
            W.setPushed(true);
        }
        else {
            RES.style = GStyleClassicResource::RAISED;
            W.setPushed(false);
        }
        drawBox(W, RES, D);
    }

    void GStyleClassic::drawText(GText&W, GStyleClassicResource&RES, VectorRenderer&D) const {
        if ((W.getWidth() == 0) && (!W.getText().empty())) {
            W.computeSize();
        }
        drawPanel(W, RES, D);
        D.setPenColor(text);
        D.drawText(W.getText(), W.getFont(), W.getRect());
    }

/*

//----------------------------------------------
    void GLayoutVector::DrawFrame(GFrame&W, GLayoutVectorResource&, VectorRenderer&, floatRES)
    {
        const uint32_t LEFTOFFSET = 8;

        Rect rect = W.Rect();

        uint32_t fh = W.Font().Height();
        uint32_t fw = W.Font().Width(W.Text());
        uint32_t l1 = rect.left;
        uint32_t t1 = rect.top+fh/2;
        uint32_t l2 = rect.left+rect.width-1;
        uint32_t t2 = rect.top+rect.height-1;
        Color c1;
        Color c2;
        switch (RES.style) {
            case GLayoutVectorResource::RAISED:
                c1 = shadowBright;
                c2 = shadowDark;
                break;
            case GLayoutVectorResource::LOWERED:
                c1 = shadowDark;
                c2 = shadowBright;
                break;
            case GLayoutVectorResource::FLAT:
                c1 = shadowDark;
                c2 = shadowDark;
                break;
        }
        if (W.Text().Len() && (W.Width() >= fw) && (W.Height() >= fh)) {
            D.SetPenColor(c1);
            D.DrawLine(l1, t1, l1+LEFTOFFSET-3, t1);
            D.DrawLine(l1+LEFTOFFSET+1+fw, t1,
                       l2, t1);
            D.SetPenColor(c2);
            D.DrawLine(l1+1, t1+1, l1+LEFTOFFSET-3, t1+1);
            D.DrawLine(l1+LEFTOFFSET+1+fw, t1+1,
                       l2-2, t1+1);
            W.Font().Draw(D, W.Text(), l1+LEFTOFFSET, t1-fh/2);
        }
        else {
            D.SetPenColor(c1);
            D.DrawLine(l1, t1, l2, t1);
            D.SetPenColor(c2);
            D.DrawLine(l1+1, t1+1, l2-2, t1+1);
        }

        D.SetPenColor(c1);
        D.DrawLine(l1, t2, l1, t1);
        D.DrawLine(l2-1, t1+1, l2-1, t2-1);
        D.DrawLine(l2-1, t2-1, l1+1, t2-1);
        D.SetPenColor(c2);
        D.DrawLine(l1+1, t2-2, l1+1, t1+1);
        D.DrawLine(l2, t1, l2, t2);
        D.DrawLine(l2, t2, l1, t2);
    }



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
    
}