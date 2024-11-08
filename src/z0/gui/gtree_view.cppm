/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GTreeView;

import z0.Application;
import z0.Constants;
import z0.Tools;

import z0.GBox;
import z0.GPanel;
import z0.GScrollBar;
import z0.GText;
import z0.GWidget;

export namespace z0 {
    class GTreeView : public GWidget {
    public:
        class Item : public GPanel {
        public:
            shared_ptr<GWidget> item;
            shared_ptr<GText> handle;
            list<shared_ptr<Item>> children;
            int level{0};
            bool selected{false};
            bool expanded{false};

            explicit Item(shared_ptr<GWidget> _item) : item{_item} {
            }
        };

        explicit GTreeView(): GWidget(TREEVIEW) {}

        void setResources(const string& RBOX, const string& RSCROLL, const string&) {
            if (box == nullptr) {
                box = make_shared<GBox>();
                vscroll = make_shared<GVScrollBar>(0, 0);
                add(vscroll, RIGHT, RSCROLL);
                add(box, FILL, RBOX);
                box->setDrawBackground(false);
                box->setPadding(1);
                vscroll->setStep(2);
            }
        }

        void removeAllItems() {
            box->removeAll();
            items.clear();
        }

        shared_ptr<Item>& addItem(shared_ptr<GWidget> item) {
            items.push_back(make_shared<Item>(item));
            auto& newWidget = items.back();
            box->add(newWidget, GWidget::TOPLEFT);
            newWidget->handle = make_shared<GText>(" ");
            newWidget->add(newWidget->handle, GWidget::LEFT, treeTabsSize);
            newWidget->add(item, GWidget::LEFT);
            newWidget->setSize(Application::get().getWindow().getWidth(), item->getHeight());
            newWidget->setDrawBackground(false);
            return newWidget;
        }

        shared_ptr<Item>& addItem(const shared_ptr<Item>& parent, shared_ptr<GWidget> item) {
            parent->children.push_back(make_shared<Item>(item));
            auto& newWidget = parent->children.back();
            newWidget->level = parent->level + 1;
            box->add(newWidget, GWidget::TOPLEFT);
            for (int i = 0; i <= newWidget->level; i++) {
                if (i == (newWidget->level)) {
                    newWidget->handle = make_shared<GText>(" ");
                    newWidget->add(newWidget->handle, GWidget::LEFT, treeTabsSize);
                }
                else {
                    newWidget->add(make_shared<GPanel>(), GWidget::LEFT, treeTabsSize);
                }
            }
            parent->handle->setText("-");
            newWidget->add(item, GWidget::LEFT);
            newWidget->setSize(Application::get().getWindow().getWidth(), item->getHeight());
            newWidget->setDrawBackground(false);
            //expand(parent->item);
            return newWidget;
        }

        void expand(const shared_ptr<GWidget>& item) const {
            for (const auto& widget : box->_getChildren()) {
                if (auto itemWidget = dynamic_cast<GTreeView::Item*>(item.get())) {
                    if (itemWidget->item == item) {
                        log("found");
                    }
                }
            }
        }

    private:
        const string treeTabsSize{"5,5"};
        float innerHeight;
        float itemsHeight;
        list<shared_ptr<Item>> items;
        shared_ptr<GBox> box;
        shared_ptr<GVScrollBar> vscroll;
    };
}
