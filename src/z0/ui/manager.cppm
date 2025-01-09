/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Manager;

import z0.Constants;
import z0.InputEvent;
import z0.Object;

import z0.resources.Font;

import z0.ui.Window;

import z0.vulkan.VectorRenderer;

export namespace z0 {

    namespace ui {
        /**
         * Manage all the UI windows
         */
        class Manager: public Object {
        public:
            /**
             * Adds a UI Window to the list of managed windows
             */
            void add(const shared_ptr<Window>&);

            /**
             * Removes a UI Window to the list of managed windows. The Window will be removed at the start of the next frame.
             */
            void remove(const shared_ptr<Window>&);

            /**
             * Returns the default font loaded at startup
             */
            [[nodiscard]] inline Font& getDefaultFont() const { return *defaultFont; }

            /**
             * Forces a redraw of all the UI at the start of the next frame
             */
            inline void refresh() { needRedraw = true; }

            [[nodiscard]] inline VectorRenderer& getRenderer() const { return *vectorRenderer; }
            [[nodiscard]] inline float getResizeDelta() const { return resizeDelta; }
            void setEnableWindowResizing(const bool enable) { enableWindowResizing = enable; }

            void drawFrame();

            [[nodiscard]] bool onInput(InputEvent& inputEvent);

        private:
            const float                 resizeDelta{5.0f};
            shared_ptr<Font>            defaultFont;
            shared_ptr<VectorRenderer>& vectorRenderer;
            list<shared_ptr<Window>>    windows;
            mutex                       windowsMutex;
            vector<shared_ptr<Window>>  removedWindows{};
            shared_ptr<Window>          focusedWindow{nullptr};
            shared_ptr<Window>          resizedWindow{nullptr};
            bool                        needRedraw{false};
            bool                        enableWindowResizing{true};
            bool                        resizingWindow{false};
            bool                        resizingWindowOriginBorder{false};
            MouseCursor                 currentCursor{MouseCursor::ARROW};

        public:
            Manager(shared_ptr<VectorRenderer>&, const string& defaultFont, uint32_t defaultFontSize);
            ~Manager() override;
        };
    }

}