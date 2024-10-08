module;
#include "z0/libraries.h"

export module z0:GTextEdit;

import :Tools;
import :Constants;
import :GWidget;
import :GBox;
import :GText;
import :GEvent;

export namespace z0 {
    class GTextEdit : public GWidget {
    public:
        explicit GTextEdit(const string& TXT = ""): GWidget(TEXTEDIT),
                                    selStart(0), selLen(0), startPos(0), nDispChar(0) {
            allowFocus = true;
            text = TXT;
        }

        ~GTextEdit() override = default;

        bool isReadOnly() const {
            return readonly;
        }

        void setReadOnly(const bool READONLY) {
            readonly = READONLY;
        }

        void setText(const string& TEXT) {
            if (text == TEXT) return;
            if (text.empty()) {
                selStart = 0;
                startPos = 0;
            }
            text = TEXT;
            computeNDispChar();
            if (parent) { parent->refresh(); }
            gtext->setText(text.substr(startPos, nDispChar + 1));
            box->refresh();
            refresh();
            auto event = GEventTextChange{.text = text};
            event.source = this;
            emit(GEvent::OnTextChange, &event);
        }

        void setSelStart(uint32_t S) {
            selStart = S;
            if (startPos > selStart) startPos = selStart;
        }

        const string& getText() const { return const_cast<string&>(text); }
        uint32_t getSelStart() const { return selStart; }
        uint32_t getFirstDisplayedChar() const { return startPos; };

        // return TRUE if this or parent have keyboard focus
        //bool haveFocus();

        string getDisplayedText() const {
            return gtext->getText();
        }

        void setResources(const string& BRES) {
            add(box, GWidget::FILL, BRES);
            box->add(gtext, GWidget::HCENTER);
            selStart = 0;
            startPos = 0;
            computeNDispChar();
        }

    protected:
        string text;
        bool readonly{false};
        uint32_t selStart;
        uint32_t selLen;
        uint32_t startPos;
        uint32_t nDispChar;
        shared_ptr<GBox> box;
        shared_ptr<GText> gtext;

        bool eventKeybDown(Key K) override {
            const auto consumed = GWidget::eventKeybDown(K);
            if (isReadOnly()) { return K; }

            setFreezed(true);
            if (K == Key::KEY_LEFT) {
                if (selStart > 0) { selStart--; }
            }
            else if (K == Key::KEY_RIGHT) {
                if (selStart < text.length()) { selStart++; }
            }
            else if (K == Key::KEY_END) {
                selStart = text.length();
            }
            else if (K == Key::KEY_HOME) {
                selStart = 0;
            }
            else if (K == Key::KEY_BACKSPACE) {
                if (selStart > 0) {
                    selStart--;
                    setText(text.substr(0, selStart) + text.substr(selStart + 1,
                                                                   text.length() - selStart - 1));
                }
            }
            else if (K == Key::KEY_DELETE) {
                if (selStart < text.length()) {
                    setText(text.substr(0, selStart) + text.substr(selStart + 1,
                                                                   text.length() - selStart - 1));
                }
            }
            /*else if ((K != keyb->KEY_SHIFTRIGHT) &&
                    (K != keyb->KEY_SHIFTLEFT) &&
                    (K != keyb->KEY_CTRLRIGHT) &&
                    (K != keyb->KEY_CTRLLEFT) &&
                    (K != keyb->KEY_ALTRIGHT) &&
                    (K != keyb->KEY_ALTLEFT))
            {
                UChar c = keyb->CodeToChar(K);
                if (c >= _WORD(0x0020)) {
                    SetText(text.Left(selStart) + c +
                            text.Right(text.Len() - selStart));
                    selStart++;
                }
                else {
                    Freeze() = FALSE;
                    return K;
                }
            }*/
            else {
                setFreezed(false);
                return consumed;
            }
            computeNDispChar();
            if (selStart < startPos) {
                startPos = selStart;
            }
            else if (((selStart + selLen) > (startPos + nDispChar)) &&
                (nDispChar != text.length())) {
                startPos = selStart - nDispChar;
            }
            computeNDispChar();
            setFreezed(false);
            gtext->setText(text.substr(startPos, nDispChar + 1));
            box->refresh();
            refresh();
            return K;
        }

        // Compute the number of displayed characters
        void computeNDispChar() {
            die("not implemented");
        }
    };
}
