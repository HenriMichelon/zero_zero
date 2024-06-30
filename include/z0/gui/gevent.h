#pragma once

namespace z0 {

    /**
     * List of widgets events signals
     */
    class GEvent {
    public:
        //! called after widget creation (all widgets)
        static const string OnCreate;		
        //! called before widget destruction (all widgets)
        static const string OnDestroy;		
        //! called when the user press a key & the widget have the keyboard focus (all widgets)
        static const string OnKeyDown;		
        //! called when the user press a key & the widget have the keyboard focus (all widgets)
        static const string OnKeyUp; 		
        //! the mouse button have been pressed above the widget or a child (all widgets)
        static const string OnMouseDown;	
        //! the mouse button have been pressed above the widget or a child (all widgets)
        static const string OnMouseUp;		
        //! the mouse have been moved above the widget (all widgets)
        static const string OnMouseMove;
        //! widget acquire keyboard focus (all widgets)
        static const string OnGotFocus;		
        //! widget lost keyboard focus (all widgets)
        static const string OnLostFocus;	
        //! called after visibility change (all widgets)
        static const string OnShow;			
        //! called before visibility change (all widgets)
        static const string OnHide;			
        //! called after state change (all widgets)
        static const string OnEnable;
        //! called after state change (all widgets)
        static const string OnDisable;
        //! text content of the widget have changed
        //static const string OnTextChange;	
        //! called when the user click on the widget (buttons)
        static const string OnClick;		
        //! a CheckWidget state changed
        static const string OnStateChange;	
        //! value of a GValueSelect widget changed
        static const string OnValueChange;	
        //! value of a GValueSelect widget changed by the user
        static const string OnValueUserChange;	
        //! range of a GValueSelect widget changed
        static const string OnRangeChange;	
        //! item list of a GList widget have changed        
        //static const string OnInsertItem;	
        //! item list of a GList widget have changed
        //static const string OnRemoveItem;	
        //! user selected an item list of a GList widget
        //static const string OnSelectItem;	
        //! a GWindow size changed
        static const string OnResize;	
        //! a GWindow position changed
        static const string OnMove;	
    };

    class GWidget;

    /**
     * Parameter for GEvent::OnClick
     */
    struct GEventClick: public Signal::Parameters {
        //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
        bool        consumed{false};
    };

    /**
     * Parameters for GEvent::OnKeyDown and GEvent::OnKeyUp
     */
    struct GEventKeyb: public Signal::Parameters {
        //! Key code
        Key	        key;
        //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
        bool        consumed{false};
    };

    /**
     * Parameters for GEvent::OnMouseDown and GEvent::OnMouseUp
     */
    struct GEventMouseButton: public Signal::Parameters {
        //! Mouse button
        MouseButton	button;
        //! X coord
        float   	x;
        //! Y coord
        float   	y;
        //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
        bool        consumed{false};
    };

    /**
     * Parameters for GEvent::OnMouseMove
     */
    struct GEventMouseMove: public Signal::Parameters {
        //! Mouse button states
        uint32_t	buttonsState;
        //! X coord
        float   	x;
        //! Y coord
        float   	y;
        //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
        bool        consumed{false};
    };

    /**
     * Parameters for GEvent::OnStateChange
     */
    struct GEventState: public Signal::Parameters {
        //! GCheckWidget::State
        int32_t	    state;
    };

    /**
     * Parameters for GEvent::GEventValue
     */
    struct GEventValue: public Signal::Parameters {
        float	    value;
        float	    previous;
    };

    /**
     * Parameters for GEvent::GEventRange
     */
    struct GEventRange: public Signal::Parameters {
        float	    min;
        float	    max;
        float	    value;
    };

    /*struct GEventText: public Signal::Parameters {
        const string	text;
    };

    struct GEventItem: public Signal::Parameters {
        int32_t	            index;
        shared_ptr<GWidget> item;
    };*/

}