#pragma once

#include "z0/object.h"
#include "z0/renderers/vector_renderer.h"
#include "z0/gui/gwindow.h"

namespace z0 {

    class Application;

    class GManager: public Object {
    public:
        explicit GManager(shared_ptr<VectorRenderer>&, const string& defaultFont, uint32_t defaultFontSize);
        ~GManager();

        void add(const shared_ptr<GWindow>&);
        void refresh() { needRedraw = true; }
        VectorRenderer& getRenderer() { return *vectorRenderer; }

    private:
        shared_ptr<Font>            defaultFont;
        shared_ptr<VectorRenderer>& vectorRenderer;
        list<shared_ptr<GWindow>>   windows;
        GWindow*                    focusedWindow{nullptr};
        bool                        needRedraw{false};

        void drawFrame();
        bool onInput(InputEvent& inputEvent);
        void windowHidden(GWindow*);
        void windowShown(GWindow*);

        friend class Application;
        friend class GWindow;
    };

}