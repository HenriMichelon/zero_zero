/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GManager;

import z0.Constants;
import z0.Font;
import z0.InputEvent;
import z0.Object;

import z0.GWindow;

import z0.VectorRenderer;

export namespace z0 {

    /**
     * Manage all the UI windows
     */
    class GManager: public Object {
    public:
        /**
         * Adds a UI window to the list of managed windows
         */
        void add(const shared_ptr<GWindow>&);

        /**
         * Removes a UI window to the list of managed windows. The window will be removed at the start of the next frame.
         */
        void remove(const shared_ptr<GWindow>&);

        /**
         * Returns the default font loaded at startup
         */
        [[nodiscard]] inline shared_ptr<Font>& getDefaultFont() { return defaultFont; }

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
        list<shared_ptr<GWindow>>   windows;
        vector<shared_ptr<GWindow>> removedWindows{};
        shared_ptr<GWindow>         focusedWindow{nullptr};
        shared_ptr<GWindow>         resizedWindow{nullptr};
        bool                        needRedraw{false};
        bool                        enableWindowResizing{true};
        bool                        resizingWindow{false};
        bool                        resizingWindowOriginBorder{false};
        MouseCursor                 currentCursor{MouseCursor::ARROW};

    public:
        GManager(shared_ptr<VectorRenderer>&, const string& defaultFont, uint32_t defaultFontSize);
        ~GManager() override;
    };

}