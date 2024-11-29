/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.TreeView;

import z0.Application;
import z0.Constants;
import z0.Tools;

import z0.ui.Box;
import z0.ui.Panel;
import z0.ui.ScrollBar;
import z0.ui.Text;
import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        class TreeView : public Widget {
        public:
            class Item : public Panel {
            public:
                shared_ptr<Widget> item;
                shared_ptr<Text> handle;
                list<shared_ptr<Item>> children;
                int level{0};
                bool selected{false};
                bool expanded{false};

                explicit Item(shared_ptr<Widget> _item) : item{_item} {
                }
            };

            explicit TreeView(): Widget(TREEVIEW) {}

            void setResources(const string& RBOX, const string& RSCROLL, const string&) {
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

            void removeAllItems() {
                box->removeAll();
                items.clear();
            }

            shared_ptr<Item>& addItem(shared_ptr<Widget> item) {
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

            shared_ptr<Item>& addItem(const shared_ptr<Item>& parent, shared_ptr<Widget> item) {
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

            void expand(const shared_ptr<Widget>& item) const {
                for (const auto& widget : box->_getChildren()) {
                    if (auto itemWidget = dynamic_cast<TreeView::Item*>(item.get())) {
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
            shared_ptr<Box> box;
            shared_ptr<VScrollBar> vscroll;
        };
    }
}
