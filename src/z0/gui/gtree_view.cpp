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
#include "z0/gui/gtext.h"
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