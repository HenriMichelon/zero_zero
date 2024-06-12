#pragma once

namespace z0 {

    class Light: public Node {
    public:
        virtual ~Light() = default;

        const vec4& getColorAndIntensity() const { return colorAndIntensity; }
        void setColorAndIntensity(vec4 color) { colorAndIntensity = color; }
        float getSpecularIntensity() const { return specularIntensity; }
        void setSpecularIntensity(float specular) { specularIntensity = specular; }
        bool getCastShadows() const { return castShadows; }
        void setCastShadow(bool cast) { castShadows = cast; }

    protected:
        explicit Light(const string nodeName);
        explicit Light(vec4 color, float specular, const string nodeName);

    private:
        vec4 colorAndIntensity {1.0f, 1.0f, 1.0f, 1.0f};
        float specularIntensity {1.0f};
        bool castShadows {false};
    };

}