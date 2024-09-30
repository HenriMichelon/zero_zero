module;
#include "z0/libraries.h"

export module Z0:Environment;

import :Tools;
import :Node;

export namespace z0 {

    /**
     * Environment properties for the scene
     */
    class Environment : public Node {
    public:
        /**
         * Creates en Environment object
         * @param colorAndIntensity Ambient RGB color and intensity
         * @param nodeName Node name.
         */
        explicit Environment(
                const vec4    colorAndIntensity = {1.0f, 1.0f, 1.0f, 1.0f},
                const string &nodeName          = "Environment"):
            Node{nodeName, ENVIRONMENT},
            ambientColorIntensity{colorAndIntensity} {
        }

        ~Environment() override = default;

        /**
         * Returns the ambient RGB color and intensity
         */
        [[nodiscard]] const vec4 &getAmbientColorAndIntensity() const { return ambientColorIntensity; }

        /**
         * Sets the ambient RGB color and intensity
        */
        void setAmbientColorAndIntensity(const vec4 ambientColorIntensity) { this->ambientColorIntensity = ambientColorIntensity; }

        void setProperty(const string &property, const string &value) override {
            Node::setProperty(property, value);
            if (property == "ambient_color") {
                setAmbientColorAndIntensity(to_vec4(value));
            }
        }

    private:
        vec4 ambientColorIntensity;
    };

}
