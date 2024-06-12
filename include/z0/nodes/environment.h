#pragma once

namespace z0 {

    class Environment : public Node {
    public:
        explicit Environment(vec4 colorAndIntensity = {1.0f, 1.0f, 1.0f, 1.0f}, const string nodeName = "Environment"): 
            Node{nodeName}, ambientColorIntensity{colorAndIntensity} {}
        virtual ~Environment() {};

        const vec4& getAmbientColorAndIntensity() const { return ambientColorIntensity; }
        void setAmbientColorAndIntensity(vec4 color) { ambientColorIntensity = color; }

    private:
        vec4 ambientColorIntensity;
    };

}