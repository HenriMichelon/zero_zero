/*
 * Copyright (c) 2024-2025 Henri Michelon
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

                explicit Item(shared_ptr<Widget> _item) : item{_item} {}
            };

            TreeView();

            void setResources(const string& RBOX, const string& RSCROLL, const string&);

            void removeAllItems();

            shared_ptr<Item>& addItem(shared_ptr<Widget> item);

            shared_ptr<Item>& addItem(const shared_ptr<Item>& parent, shared_ptr<Widget> item);

            void expand(const shared_ptr<Widget>& item) const;

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
