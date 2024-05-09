#include "z0/gui/gmanager.h"

namespace z0 {

    GManager::GManager(shared_ptr<z0::VectorRenderer> &renderer): vectorRenderer{renderer}{

    }

    void GManager::refresh() {
        vectorRenderer->beginDraw();
        for (auto& window: windows) {
            vectorRenderer->setPenColor(window->getBgColor().color);
            vectorRenderer->setTransparency(window->getBgColor().color.a);
            auto& rect = window->getRect();
            vec2 topLeft = vec2{rect.left, rect.top} / SCALE;
            vec2 bottomRight = vec2{rect.left+rect.width, rect.top-rect.height} / SCALE;
            vectorRenderer->drawFilledRect(topLeft, bottomRight);
        }
        vectorRenderer->endDraw();
    }

    void GManager::add(shared_ptr<z0::GWindow> &window) {
        windows.push_back(window);
    }

}