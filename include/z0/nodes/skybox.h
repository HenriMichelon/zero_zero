#pragma once

namespace z0 {

    class Skybox: public Node {
    public:
        explicit Skybox(const string& filename, const string& fileext, const string& nodeName = "Skybox");
        ~Skybox() override = default;

        shared_ptr<Cubemap>& getCubemap() { return cubemap; }

    private:
        shared_ptr<Cubemap> cubemap;
    };

}