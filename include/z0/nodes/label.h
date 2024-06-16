#pragma once

namespace z0 {

    class Label : public Node {
    public:
        Label(const string& text, const string& name);

    private:
        string  text;
        vec4    color{0.0f, 0.0f, 0.0f, 1.0f};
    };

}