#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/gui/gevent.h"
#endif

namespace z0 {

    const string GEvent::OnCreate{"on_create"};
    const string GEvent::OnDestroy{"on_destroy"};
    const string GEvent::OnKeyDown{"on_key_down"};
    const string GEvent::OnKeyUp{"on_key_up"};
    const string GEvent::OnMouseDown{"on_mouse_down"};
    const string GEvent::OnMouseUp{"on_mouse_up"};
    const string GEvent::OnMouseMove{"on_mouse_move"};
    const string GEvent::OnGotFocus{"on_got_focus"};
    const string GEvent::OnLostFocus{"on_lost_focus"};
    const string GEvent::OnShow{"on_show"};
    const string GEvent::OnHide{"on_hide"};
    const string GEvent::OnEnable{"on_enable"};
    const string GEvent::OnDisable{"on_disable"};
    //const string GEvent::OnTextChange{"on_text_change"};
    const string GEvent::OnClick{"on_click"};
    const string GEvent::OnStateChange{"on_state_change"};
    const string GEvent::OnResize{"on_resize"};
    const string GEvent::OnMove{"on_move"};
    const string GEvent::OnValueChange{"on_value_change"};
    //const string GEvent::OnValueUserChange{"on_value_use_change"};
    const string GEvent::OnRangeChange{"on_range_change"};
/*     const string GEvent::OnInsertItem{"on_insert_item"};
    const string GEvent::OnRemoveItem{"on_remove_item"};
    const string GEvent::OnSelectItem{"on_select_item"};
 */
}