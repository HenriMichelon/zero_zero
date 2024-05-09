#include "z0/tools.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gwindow.h"

#include <ranges>
namespace z0 {


//------------------------------------------------------------
    GWindow::GWindow(): 
                        mWidget(nullptr), mFocusedWidget(nullptr)
    {
        mFreeze = false;
        mLayout = nullptr;
    }


//------------------------------------------------------------
    void GWindow::unFreeze(shared_ptr<GWidget>&W)
    {
        for (auto& child: W->getChildren()) {
            unFreeze(child);
        }
        W->isFreezed() = false;
    }

//------------------------------------------------------------
    GWidget& GWindow::setWidget(shared_ptr<GWidget>WIDGET, const string&RES, uint32_t PADDING)
    {
        if (mLayout == nullptr) { setLayout(nullptr); }
        if (WIDGET == nullptr) {
            mWidget = make_shared<GPanel>();
        }
        else {
            mWidget = std::move(WIDGET);
        }
        mWidget->mFreeze = true;
        mWidget->mPadding = PADDING;
        mWidget->window = this;
        mWidget->layout = mLayout;
        mWidget->font = mWidget->layout->getFont();
        mWidget->layout->addResource(*mWidget, RES);
        mWidget->setDrawBackground(true);
        mWidget->eventCreate();
        mWidget->setPos(0, 0);
        mWidget->setSize(getWidth(), getHeight());
        mFocusedWidget = mWidget->setFocus();
        unFreeze(mWidget);
        return *mWidget;
    }


//------------------------------------------------------------
    void GWindow::setLayout(shared_ptr<GLayout> LAYOUT)
    {
        mLayout = std::move(LAYOUT);
        if (mLayout == nullptr) { mLayout = GLayout::create(); }

        auto opt = mLayout->getOption("color_background");
        if (!opt.empty()) {
            auto rgb = split(opt, ',');
            if (rgb.size() == 3) {
                auto red = stof(string(rgb[0]));
                auto green = stof(string(rgb[0]));
                auto blue = stof(string(rgb[0]));
                setBgColor(Color(red, green, blue));
            }
        }
    }


//------------------------------------------------------------
    void GWindow::eventCreate()
    {
        onCreate();
        if (mWidget == nullptr) { setWidget(); }
        if (mWidget != nullptr ) { mWidget->resizeChildren(); }
    }


//------------------------------------------------------------
    void GWindow::eventDestroy()
    {
        if (mWidget) { mWidget->eventDestroy(); }
        onDestroy();
    }


//------------------------------------------------------------
    bool GWindow::eventQueryDestroy()
    {
        return onQueryDestroy();
    }


//------------------------------------------------------------
    void GWindow::eventDraw(const GRect&R)
    {
        if (mFreeze) { return; }
        if (mWidget) { mWidget->refresh(); }
        onDraw(R);
        endRefresh();
    }


//------------------------------------------------------------
    void GWindow::eventShow()
    {
        if (mWidget) { mWidget->eventShow(); }
        onShow();
        endRefresh();
    }


//------------------------------------------------------------
    void GWindow::eventKeybDown(Key K)
    {
/*	if ((K == keyb.KEY_TAB) && mWidget)
	{
//		mWidget->ClosePopup();
		if (mFocusedWidget)
			mFocusedWidget = mFocusedWidget->SetNextFocus();
		else
			mFocusedWidget = mWidget->SetFocus();
	}
	else*/ if (mFocusedWidget != nullptr) {
            mFocusedWidget->eventKeybDown(K);
        }
        onKeybDown(K);
        //if (!display->NativeDoubleBuffer()) {
            endRefresh();
        //}
    }


//------------------------------------------------------------
    void GWindow::eventKeybUp(Key K)
    {
        if (mFocusedWidget != nullptr) mFocusedWidget->eventKeybUp(K);
        onKeybUp(K);
        //if (handle == nullptr) { return; }
        //if (!display->NativeDoubleBuffer()) {
            endRefresh();
        //}
    }


//------------------------------------------------------------
    void GWindow::eventMouseDown(MouseButton B, int32_t X, int32_t Y)
    {
        if (mWidget) {
            shared_ptr<GWidget>newfocused = mWidget->eventMouseDown(B, X, Y);
            //if (handle == nullptr) { return; }
            if (newfocused.get() != mFocusedWidget) {
                if (mFocusedWidget != nullptr) { mFocusedWidget->setFocus(false); }
                mFocusedWidget = newfocused.get();
            }
        }
        onMouseDown(B, X, Y);
        //if (handle == nullptr) { return; }
        //if (!display->NativeDoubleBuffer()) {
        endRefresh();
        //}
    }


//------------------------------------------------------------
    void GWindow::eventMouseUp(MouseButton B, int32_t X, int32_t Y)
    {
        if (mWidget) {
            mWidget->eventMouseUp(B, X, Y);
            //if (handle == nullptr) { return; }
        }
        onMouseUp(B, X, Y);
        //if (handle == nullptr) { return; }
        //if (!display->NativeDoubleBuffer()) {
        endRefresh();
        //}
    }


//------------------------------------------------------------
    void GWindow::eventMouseMove(MouseButton B, int32_t X, int32_t Y)
    {
        if ((mFocusedWidget != nullptr) &&
            (mFocusedWidget->mouseMoveOnFocus)) {
            mFocusedWidget->eventMouseMove(B, X, Y);
        }
        else if (mWidget) {
            mWidget->eventMouseMove(B, X, Y);
        }
        //if (handle == nullptr) { return; }
        onMouseMove(B, X, Y);
        //if (handle == nullptr) { return; }
        //if (!display->NativeDoubleBuffer()) {
            endRefresh();
        //}
    }


//------------------------------------------------------------
    void GWindow::eventGotFocus()
    {
        onGotFocus();
    }


//------------------------------------------------------------
    void GWindow::eventLostFocus()
    {
        onLostFocus();
    }


//------------------------------------------------------------
    void GWindow::endRefresh()
    {
        if (mFreeze) { return; }
        mFreeze = true;
        if (isVisible() && mWidget) {
            startRefresh();
            mWidget->flushRefresh(mRefreshrect);
            if (mRefreshrect.width && mRefreshrect.height) {
                //refreshDisplay(mRefreshrect); // XXX
            }
        }
        mFreeze = false;
    }


//------------------------------------------------------------
    void GWindow::startRefresh()
    {
        mRefreshrect.left = mRefreshrect.top =
        mRefreshrect.width = mRefreshrect.height = 0;
    }


//------------------------------------------------------------
    void GWindow::refresh()
    {
        if (mWidget) { mWidget->refresh(); }
        endRefresh();
    }


//------------------------------------------------------------
    void GWindow::setFocusedWidget(const shared_ptr<GWidget>& W)
    {
        mFocusedWidget = W.get();
    }


//------------------------------------------------------------
    GWidget& GWindow::getWidget() {
        return *mWidget;
    };


//------------------------------------------------------------
    shared_ptr<GLayout> GWindow::getLayout() const {
        return mLayout;
    };


//------------------------------------------------------------
    void GWindow::eventHide() {
        onHide();
    };


}