#pragma once

#include <utility>

#include "z0/constants.h"

namespace z0 {

    class GEvent {
    public:
        //! Event list
        enum Type {
            OnCreate,		//! called after widget creation (all widgets)
            OnDestroy,		//! called before widget destruction (all widgets)
            OnDraw,			//! called after widget drawing (all widgets)
            OnKeybDown,		//! called when the user press a key & the widget have the keyboard focus (all widgets)
            OnKeybUp, 		//! called when the user press a key & the widget have the keyboard focus (all widgets)
            OnMouseDown,	//! the mouse button have been pressed above the widget or a child (all widgets)
            OnMouseUp,		//! the mouse button have been pressed above the widget or a child (all widgets)
            OnMouseMove,	//! the mouse have been moved above the widget (all widgets)
            OnResize,		//! the widget size have changed (all widgets)
            OnMove,			//! the widget position have changed (all widgets)
            OnGotFocus,		//! widget acquire keyboard focus (all widgets)
            OnLostFocus,	//! widget lost keyboard focus (all widgets)
            OnShow,			//! called after visibility change (all widgets)
            OnHide,			//! called before visibility change (all widgets)
            OnEnable,
            OnDisable,
            OnTextChange,	//! text content of the widget have changed
            OnClick,		//! the user click above the widget
            OnStateChange,	//! a CheckWidget state changed
            OnPictureChange,//! pixmap of a GPicture changed
            OnInsertItem,	//! item list of a GList widget have changed
            OnRemoveItem,	//! item list of a GList widget have changed
            OnSelectItem,	//! user selected an item list of a GList widget
            OnValueChange,	//! value of a GValueSelect widget changed
            OnValueUserChange,	//! value of a GValueSelect widget changed by the user
            OnRangeChange,	//! range of a GValueSelect widget changed
            nbEvents
        } ;
    };


//-------------------------------------------------------
    class GEventKeyb: public GEvent
    {
    public:
        Key	key;

        GEventKeyb(Key K): key(K) { };
    };


//-------------------------------------------------------
    class GEventMouse: public GEvent
    {
    public:
        MouseButton	button;
        uint32_t	x;
        uint32_t	y;

        GEventMouse(MouseButton B, uint32_t X, uint32_t Y):
                button(B), x(X), y(Y) {};
        GEventMouse() = default;
    };


//-------------------------------------------------------
    class GEventSize: public GEvent
    {
    public:
        uint32_t	width;
        uint32_t	height;

        GEventSize(uint32_t W, uint32_t H): width(W), height(H) {};
    };


//-------------------------------------------------------
    class GEventPos: public GEvent
    {
    public:
        int32_t	top;
        int32_t	left;

        GEventPos(int32_t T, int32_t L): top(T), left(L) {};
    };


//-------------------------------------------------------
    class GEventValue: public GEvent
    {
    public:
        int32_t	value;
        int32_t	previous;

        explicit GEventValue(int32_t V = 0, int32_t P = 0): value(V), previous(P)  {};
    };


//-------------------------------------------------------
    class GEventRange: public GEvent
    {
    public:
        int32_t	min;
        int32_t	max;
        int32_t	value;

        explicit GEventRange(int32_t I = 0, int32_t A = 0, int32_t V = 0):
                value(V) { min = I; max = A; };
    };


//-------------------------------------------------------
    class GEventState: public GEvent
    {
    public:
        int32_t	state;
        explicit GEventState(int32_t S = 0): state(S) {};
    };


//-------------------------------------------------------
    class GEventText: public GEvent
    {
    public:
        const string	text;
        explicit GEventText(string  T = ""): text(std::move(T)) {};
        virtual ~GEventText() = default;
    };


//-------------------------------------------------------
    /*class GEventPicture: public GEvent
    {
    public:
        IPixmap	*pixmap;
    };*/

    class GWidget;

//-------------------------------------------------------
    class GEventItem: public GEvent
    {
    public:
        int32_t	index;
        shared_ptr<GWidget> item;

        GEventItem(int32_t I, shared_ptr<GWidget> S): index(I), item(std::move(S)) {};
        virtual ~GEventItem() = default;
    };

}