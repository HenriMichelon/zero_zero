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

    void GTreeView::setResources(const string&RBOX, const string&RSCROLL, const string&) {
         if (box == nullptr) {
            box = make_shared<GBox>();
            vscroll = make_shared<GVScrollBar>(0,0);
            add(vscroll, RIGHT, RSCROLL);
            add(box, FILL, RBOX);
            box->setDrawBackground(false);
            box->setPadding(0);
	        vscroll->setStep(2);
         }
    }

}