#include "z0/base.h"
#include "z0/nodes/node.h"

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(string nodeName):
        name{std::move(nodeName)},
        id{currentId++}   {
        replace(name.begin(), name.end(),  '/', '_');
        localTransform = mat4 {1.0};
        updateTransform(mat4{1.0f});
    }

    Node::Node(const Node& orig): id{currentId++} {
        name = orig.name;
        parent = orig.parent;
        localTransform = orig.localTransform;
        worldTransform = orig.worldTransform;
        processMode = orig.processMode;
    }

    void Node::translate(vec3 localOffset) {
        // current orientation * velocity
        vec3 worldTranslation =  toQuat(mat3(localTransform)) * localOffset;
        setPosition(getPosition() + worldTranslation);
    }

    void Node::setPosition(vec3 pos) {
        localTransform[3] = vec4(pos, 1.0f);
        updateTransform(mat4{1.0f});
    }

    void Node::setPositionGlobal(vec3 pos) {
        if (parent == nullptr) {
            setPosition(pos);
            return;
        }
        auto inverseParentTransform = inverse(parent->worldTransform);
        auto newLocalPositionHomogeneous = inverseParentTransform * vec4(pos, 1.0);
        auto newLocalPosition = vec3(newLocalPositionHomogeneous);
        localTransform[3] = vec4(newLocalPosition, 1.0f);
        updateTransform(mat4{1.0f});
    }

    void Node::updateTransform(const mat4& parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::updateTransform() {
        auto parentMatrix = parent == nullptr ? mat4{1.0f} : parent->worldTransform;
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::rotateX(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_X);
        updateTransform();
    }

    void Node::rotateY(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Y);
        updateTransform();
    }

    void Node::rotateZ(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Z);
        updateTransform();
    }

    void Node::setRotation(quat quater) {
        vec3 scale, translation, skew;
        vec4 perspective;
        quat orientation;
        // Decompose the original matrix to extract translation, rotation (orientation), and scale
        decompose(localTransform, scale, orientation, translation, skew, perspective);
        // Create a rotation matrix from the new quaternion
        mat4 rotationMatrix = toMat4(quater);
        // Reconstruct the transformation matrix with the new rotation, preserving the original translation and scale
        localTransform = glm::translate(mat4(1.0f), translation)
                         * rotationMatrix
                         * glm::scale(mat4(1.0f), scale);
        updateTransform();
    }

    vec3 Node::getRotation() const {
        return eulerAngles(toQuat(mat3(localTransform)));
    };

    void Node::setRotationX(float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::setScale(float scale) {
        setScale(vec3{scale, scale, scale});
    }

    void Node::setScale(glm::vec3 scale) {
        localTransform = glm::scale(localTransform, scale);
        updateTransform();
    }

    glm::vec3 Node::getScale() const {
        vec3 scale;
        quat rotation;
        vec3 translation;
        vec3 skew;
        vec4 perspective;
        decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    void Node::_onReady() {
        inReady = true;
        onReady();
        inReady = false;
    }

    shared_ptr<Node> Node::getChild(const string& name) {
        auto it = std::find_if(children.begin(), children.end(), [name](std::shared_ptr<Node> elem) {
            return elem->name == name;
        });
        return it == children.end() ? nullptr : *it;
    }

    shared_ptr<Node> Node::getNode(const string& path) {
        size_t pos = path.find('/');
        if (pos != std::string::npos) {
            auto child = getChild(path.substr(0, pos));
            if (child != nullptr) {
                return child->getNode(path.substr(pos + 1));
            }
            return nullptr;
        } else {
            return getChild(path);
        }
    }

    bool Node::addChild(const shared_ptr<Node>& child) {
        if (haveChild(child)) return false;
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
        if (inReady) child->_onReady();
        if (addedToScene) Application::get().addNode(child);
        return true;
    }

    bool Node::removeChild(const shared_ptr<Node>& node) {
        if (!haveChild(node)) return false;
        children.remove(node);
        node->parent = nullptr;
        Application::get().removeNode(node);
        return true;
    }

    bool Node::haveChild(const shared_ptr<z0::Node> &child, bool recursive) {
        if (recursive) {
            if (haveChild(child)) return true;
            for(const auto& node : children) {
                if (node->haveChild(child, true)) return true;
            }
            return false;
        } else {
            return find(children.begin(), children.end(), child) != children.end();
        }
    }

    shared_ptr<Node> Node::duplicate() {
        shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for(auto&child : children) {
            dup->addChild(child->duplicate());
        }
        dup->id = currentId++;
        dup->name = name;
        return dup;
    }

    shared_ptr<Node> Node::duplicateInstance() {
        return make_shared<Node>(*this);
    }

    void Node::printTree(ostream& out, int tab) {
        for (int i = 0; i < (tab*2); i++) {
            out << " ";
        }
        out << toString() << std::endl;
        for (auto& child: children) child->printTree(out, tab+1);
    }

    bool Node::isProcessed() const {
        const auto paused = Application::get().isPaused();
        auto mode = processMode;
        if ((parent == nullptr) && (mode == PROCESS_MODE_INHERIT)) mode = PROCESS_MODE_PAUSABLE;
        return ((mode == PROCESS_MODE_INHERIT) && (parent->isProcessed())) ||
               (!paused && (mode == PROCESS_MODE_PAUSABLE)) ||
               (paused && (mode == PROCESS_MODE_WHEN_PAUSED)) ||
               (mode == PROCESS_MODE_ALWAYS);
    }

}