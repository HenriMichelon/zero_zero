#include "z0/nodes/node.h"

#include <algorithm>
#include <utility>

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(string nodeName): name{std::move(nodeName)}, id{currentId++}   {
        replace(name.begin(), name.end(),  '/', '_');
    }

    Node::Node(const Node& orig): id{currentId++} {
        name = orig.name;
    }

}