#pragma once

namespace z0 {

    /**
     * Helper class for loading resources
     */
    class Loader {
    public:
        /**
         * Load a glTF scene
         * @param filepath path of the glTF file, relative to the application path
         * @param forceBackFaceCulling set the z0::CullMode to CULLMODE_BACK even if the material in double sided (default is CULLMODE_DISABLED for double sided materials)
         */
        static shared_ptr<Node> loadModelFromFile(const filesystem::path& filepath, bool forceBackFaceCulling = false);
    };
}
