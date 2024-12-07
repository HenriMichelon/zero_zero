/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.TextEdit;

import z0.Tools;
import z0.Constants;

import z0.ui.Box;
import z0.ui.Event;
import z0.ui.Text;
import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        class TextEdit : public Widget {
        public:
            explicit TextEdit(const string& TXT = ""): Widget(TEXTEDIT),
                                        selStart(0), selLen(0), startPos(0), nDispChar(0) {
                allowFocus = true;
                text = TXT;
            }

            ~TextEdit() override = default;

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
                auto event = EventTextChange{.text = text};
                event.source = this;
                emit(Event::OnTextChange, &event);
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
                add(box, Widget::FILL, BRES);
                box->add(gtext, Widget::HCENTER);
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
            shared_ptr<Box> box;
            shared_ptr<Text> gtext;

            bool eventKeybDown(Key K) override {
                const auto consumed = Widget::eventKeybDown(K);
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
                /*else if ((K != keyb->Key::SHIFTRIGHT) &&
                        (K != keyb->Key::SHIFTLEFT) &&
                        (K != keyb->Key::CTRLRIGHT) &&
                        (K != keyb->Key::CTRLLEFT) &&
                        (K != keyb->Key::ALTRIGHT) &&
                        (K != keyb->Key::ALTLEFT))
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
}
