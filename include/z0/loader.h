#pragma once

namespace z0 {

    class Loader {
    public:
        static shared_ptr<Node> loadModelFromFile(const filesystem::path& filepath, bool forceBackFaceCulling = false);
    };
}
