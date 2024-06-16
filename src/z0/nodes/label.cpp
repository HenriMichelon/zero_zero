#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/label.h"
#endif

namespace z0 {

    Label::Label(const string& text, const string& name):
        Node{name}
        { }

}