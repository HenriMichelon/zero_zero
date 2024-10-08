module;
#include "z0/libraries.h"

export module z0:Environment;

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
                vec4          colorAndIntensity = {1.0f, 1.0f, 1.0f, 1.0f},
                const string &nodeName          = "Environment");

        ~Environment() override = default;

        /**
         * Returns the ambient RGB color and intensity
         */
        [[nodiscard]] inline const vec4 &getAmbientColorAndIntensity() const { return ambientColorIntensity; }

        /**
         * Sets the ambient RGB color and intensity
        */
        inline void setAmbientColorAndIntensity(const vec4 ambientColorIntensity) {
            this->ambientColorIntensity = ambientColorIntensity;
        }

        void setProperty(const string &property, const string &value) override;

    private:
        vec4 ambientColorIntensity;
    };

}
