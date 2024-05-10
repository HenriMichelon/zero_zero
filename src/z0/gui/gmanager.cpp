#include "z0/gui/gmanager.h"

namespace z0 {

    GManager::GManager(shared_ptr<VectorRenderer> &renderer): vectorRenderer{renderer}{

    }

    void GManager::drawFrame() {
        if (!needRedraw) return;
        needRedraw = false;
        vectorRenderer->beginDraw();
        for (auto& window: windows) {
            window->draw(*vectorRenderer);
        }
        vectorRenderer->endDraw();
    }

    void GManager::add(const shared_ptr<GWindow> &window) {
        windows.push_back(window);
        window->windowManager = this;
        needRedraw = true;
    }

}