#pragma once

namespace z0 {

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
            vec4 colorAndIntensity = {1.0f, 1.0f, 1.0f, 1.0f}, 
            const string nodeName = "Environment"): 
            Node{nodeName}, 
            ambientColorIntensity{colorAndIntensity} {}
        virtual ~Environment() {};

        /**
         * Returns the ambient RGB color and intensity
         */
        const vec4& getAmbientColorAndIntensity() const { return ambientColorIntensity; }

        /**
         * Sets the ambient RGB color and intensity
        */
        void setAmbientColorAndIntensity(vec4 color) { ambientColorIntensity = color; }

        void setProperty(const string&property, const string& value) override;

    private:
        vec4 ambientColorIntensity;
    };

}