/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.TreeView;

namespace z0 {

    namespace ui {

        TreeView::TreeView(): Widget(TREEVIEW) {}

        void TreeView::setResources(const string& RBOX, const string& RSCROLL, const string&) {
            if (box == nullptr) {
                box = make_shared<Box>();
                vscroll = make_shared<VScrollBar>(0.0f, 0.0f);
                add(vscroll, RIGHT, RSCROLL);
                add(box, FILL, RBOX);
                box->setDrawBackground(false);
                box->setPadding(1);
                vscroll->setStep(2);
            }
        }

        void TreeView::removeAllItems() {
            box->removeAll();
            items.clear();
        }

        shared_ptr<TreeView::Item>& TreeView::addItem(shared_ptr<Widget> item) {
            items.push_back(make_shared<Item>(item));
            auto& newWidget = items.back();
            box->add(newWidget, Widget::TOPLEFT);
            newWidget->handle = make_shared<Text>(" ");
            newWidget->add(newWidget->handle, Widget::LEFT, treeTabsSize);
            newWidget->add(item, Widget::LEFT);
            newWidget->setSize(1000.0f, item->getHeight());
            newWidget->setDrawBackground(false);
            return newWidget;
        }

        shared_ptr<TreeView::Item>& TreeView::addItem(const shared_ptr<Item>& parent, shared_ptr<Widget> item) {
            parent->children.push_back(make_shared<Item>(item));
            auto& newWidget = parent->children.back();
            newWidget->level = parent->level + 1;
            box->add(newWidget, Widget::TOPLEFT);
            for (int i = 0; i <= newWidget->level; i++) {
                if (i == (newWidget->level)) {
                    newWidget->handle = make_shared<Text>(" ");
                    newWidget->add(newWidget->handle, Widget::LEFT, treeTabsSize);
                }
                else {
                    newWidget->add(make_shared<Panel>(), Widget::LEFT, treeTabsSize);
                }
            }
            parent->handle->setText("-");
            newWidget->add(item, Widget::LEFT);
            newWidget->setSize(1000.0f, item->getHeight());
            newWidget->setDrawBackground(false);
            //expand(parent->item);
            return newWidget;
        }

        void TreeView::expand(const shared_ptr<Widget>& item) const {
            for (const auto& widget : box->_getChildren()) {
                if (const auto itemWidget = dynamic_cast<TreeView::Item*>(item.get())) {
                    if (itemWidget->item == item) {
                        log("found");
                    }
                }
            }
        }

    }
}
