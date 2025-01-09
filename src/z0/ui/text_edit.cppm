/*
 * Copyright (c) 2024-2025 Henri Michelon
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
            explicit TextEdit(const string& TXT = "");

            ~TextEdit() override = default;

            inline bool isReadOnly() const { return readonly; }

            void setReadOnly(const bool READONLY) { readonly = READONLY; }

            void setText(const string& TEXT);

            void setSelStart(uint32_t S);

            const string& getText() const { return const_cast<string&>(text); }

            uint32_t getSelStart() const { return selStart; }

            uint32_t getFirstDisplayedChar() const { return startPos; }

            // return TRUE if this or parent have keyboard focus
            //bool haveFocus();

            string getDisplayedText() const { return gtext->getText(); }

            void setResources(const string& BRES);

        protected:
            string text;
            bool readonly{false};
            uint32_t selStart;
            uint32_t selLen;
            uint32_t startPos;
            uint32_t nDispChar;
            shared_ptr<Box> box;
            shared_ptr<Text> gtext;

            bool eventKeybDown(Key K) override;

            // Compute the number of displayed characters
            void computeNDispChar() {
                die("not implemented");
            }
        };
    }
}
