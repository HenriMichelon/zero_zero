module;
#include "z0/libraries.h"

export module z0:Skybox;

import :Node;
import :Cubemap;

export namespace z0 {

    /**
     * Cubemap based skybox
     */
    class Skybox : public Node {
    public:
        /**
         * Creates a Skybox based on 6 images.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filename path and filename (without the extension) of the images
         * @param fileext files extension
         */
        Skybox(const string &filename, const string &fileext);

        /**
         * Creates a Skybox from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  back  right  front`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        explicit Skybox(const string &filename);

        /**
         * Creates an empty Skybox 
         * @param filename path of the image
         */
        Skybox() = default;

        ~Skybox() override = default;

        /**
         * Load a Cubemap for the Skybox from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  back  right  front`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        void setCubemapFromFile(const string &filename);

        /**
         * Return the associated Cubemap
         */
        [[nodiscard]] inline shared_ptr<Cubemap> &getCubemap() { return cubemap; }

        void setProperty(const string &property, const string &value) override ;

    private:
        shared_ptr<Cubemap> cubemap;
    };

}
