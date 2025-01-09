/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.TextEdit;

namespace z0 {
    namespace ui {

        TextEdit::TextEdit(const string& TXT ): Widget(TEXTEDIT),
                                    selStart(0), selLen(0), startPos(0), nDispChar(0) {
            allowFocus = true;
            text = TXT;
        }

        void TextEdit::setText(const string& TEXT) {
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

        void TextEdit::setSelStart(uint32_t S) {
            selStart = S;
            if (startPos > selStart) startPos = selStart;
        }

        void TextEdit::setResources(const string& BRES) {
            add(box, Widget::FILL, BRES);
            box->add(gtext, Widget::HCENTER);
            selStart = 0;
            startPos = 0;
            computeNDispChar();
        }

        bool TextEdit::eventKeybDown(Key K) {
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
;
    }
}
