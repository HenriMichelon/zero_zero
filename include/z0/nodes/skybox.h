#pragma once

namespace z0 {

    /**
     * Cubemap based skybox
     */
    class Skybox: public Node {
    public:
        /**
         * Creates a Skybox based on 6 images.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filename path and filename (without the extension) of the images
         * @param filext files extension
         */
        explicit Skybox(const string& filename, const string& fileext);

        /**
         * Loads the cubemap from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  back  right  front`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        explicit Skybox(const string& filename);
        
        ~Skybox() override = default;

        /**
         * Return the associated Cubemap
         */
        shared_ptr<Cubemap>& getCubemap() { return cubemap; }

    private:
        shared_ptr<Cubemap> cubemap;
    };

}