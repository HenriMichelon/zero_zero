#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbox.h"
#include "z0/gui/gvalue_select.h"
#include "z0/gui/gscroll_bar.h"
#include "z0/gui/gtree_view.h"
#include "z0/application.h"
#endif

namespace z0 {

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
        newWidget->setDrawBackground(false);
        box->setSize(100, item->getHeight());
        newWidget->add(item, GWidget::LEFT);
        return newWidget;
    }

    shared_ptr<GTreeView::Item>& GTreeView::addItem(shared_ptr<Item>&parent, shared_ptr<GWidget> item) {
        parent->children.push_back(make_shared<Item>(item));
        auto& newWidget = parent->children.back();
        box->add(newWidget, GWidget::TOPLEFT);
        box->setSize(100, item->getHeight());
        newWidget->add(item, GWidget::LEFT);
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