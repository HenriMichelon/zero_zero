module;
#include "z0/modules.h"

export module Z0:GTreeView;

import :Tools;
import :Constants;
import :GWidget;
import :GPanel;
import :GBox;
import :GScrollBar;
import :GWidget;
import :GText;

export namespace z0 {

    class GTreeView : public GWidget {
    public:
       
        class Item : public GPanel {
        public:
            shared_ptr<GWidget>     item;
            shared_ptr<GText>       handle;
            list<shared_ptr<Item>> 	children;
            int                     level{0};
            bool                    selected{false};
            bool                    expanded{false};

            Item(shared_ptr<GWidget> item);
        };

        GTreeView();

        void setResources(const string&, const string&, const string&);
        void removeAllItems();
        shared_ptr<Item>& addItem(shared_ptr<GWidget> item);
        shared_ptr<Item>& addItem(shared_ptr<Item>& parent, shared_ptr<GWidget> item);
        void expand(shared_ptr<GWidget>& item);

    private:
        const string            treeTabsSize{"5,5"};
    	float		            innerHeight;
	    float		            itemsHeight;
	    list<shared_ptr<Item>> 	items;
        shared_ptr<GBox>	    box;
	    shared_ptr<GVScrollBar>	vscroll;

    };

      GTreeView::GTreeView(): GWidget(TREEVIEW) {}

    GTreeView::Item::Item(shared_ptr<GWidget> _item) : item{_item} {
    }

    void GTreeView::expand(shared_ptr<GWidget>& item) {
        for(const auto &widget : box->_getChildren()) {
            if (auto itemWidget = dynamic_cast<GTreeView::Item*>(item.get())) {
                if (itemWidget->item == item) {
                    log("found");
                }
            }
        }
    }

    shared_ptr<GTreeView::Item>& GTreeView::addItem(shared_ptr<GWidget> item) {
        items.push_back(make_shared<Item>(item));
        auto& newWidget = items.back();
        box->add(newWidget, GWidget::TOPLEFT);
        newWidget->handle = make_shared<GText>(" ");
        newWidget->add(newWidget->handle, GWidget::LEFT, treeTabsSize);
        newWidget->add(item, GWidget::LEFT);
        newWidget->setSize(app().getWindow().getWidth(), item->getHeight());
        newWidget->setDrawBackground(false);
        return newWidget;
    }

    shared_ptr<GTreeView::Item>& GTreeView::addItem(shared_ptr<Item>&parent, shared_ptr<GWidget> item) {
        parent->children.push_back(make_shared<Item>(item));
        auto& newWidget = parent->children.back();
        newWidget->level = parent->level + 1;
        box->add(newWidget, GWidget::TOPLEFT);
        for (int i = 0; i <= newWidget->level; i++) {
            if (i == (newWidget->level)) {
                newWidget->handle = make_shared<GText>(" ");
                newWidget->add(newWidget->handle, GWidget::LEFT, treeTabsSize);
            } else {
                newWidget->add(make_shared<GPanel>(), GWidget::LEFT, treeTabsSize);
            }
        }
        parent->handle->setText("-");
        newWidget->add(item, GWidget::LEFT);
        newWidget->setSize(app().getWindow().getWidth(), item->getHeight());
        newWidget->setDrawBackground(false);
        //expand(parent->item);
        return newWidget;
    }

    void GTreeView::removeAllItems() {
        box->removeAll();
        items.clear();
    }

    void GTreeView::setResources(const string&RBOX, const string&RSCROLL, const string&) {
         if (box == nullptr) {
            box = make_shared<GBox>();
            vscroll = make_shared<GVScrollBar>(0,0);
            add(vscroll, RIGHT, RSCROLL);
            add(box, FILL, RBOX);
            box->setDrawBackground(false);
            box->setPadding(1);
	        vscroll->setStep(2);
         }
    }

}