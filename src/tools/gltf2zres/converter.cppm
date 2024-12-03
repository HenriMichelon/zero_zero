/*
 * Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "z0/libraries.h"

export module converter;

import miplevel;

// GPU based (OpenGL) image BCn compression.
// Warning : only ONE Converter objet per thread
export class Converter {
public:
    // Initialize the GLFW3 conversion window & associated OpenGL resources
    Converter();
    // Close the window and release OpenGL resource
    ~Converter();

    // Compress one mipmaps level of an image
    bool convert(const MipLevel& inMipLevel,
                 MipLevel& outMipLevel,
                 uint32_t srcChannels,
                 const string& dstFormat) const;

    /*bool getMipMaps(vector<MipLevel>& inMipLevels,
                    uint32_t srcWidth,
                    uint32_t srcHeight,
                    uint32_t srcChannels,
                    const uint8_t* srcData) const;*/

private:
    // OpenGL equivalence table for compression formats
    const map<string, GLuint> formats = {
        { "bc1",  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT},
        { "bc2",  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT},
        { "bc3",  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT},
        { "bc4",  GL_COMPRESSED_RED_RGTC1_EXT},
        { "bc4s", GL_COMPRESSED_SIGNED_RED_RGTC1_EXT},
        { "bc5",  GL_COMPRESSED_RED_GREEN_RGTC2_EXT},
        { "bc5s", GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT},
        { "bc7",  GL_COMPRESSED_RGBA_BPTC_UNORM},
    };

    // Window shaders & buffers
    GLuint w_vertex{};
    GLuint w_fragment{};
    GLuint w_program{};
    GLuint w_vao_ref{};
    GLuint w_vbo_ref{};

    // Conversion shaders & buffers
    GLuint m_vertex{};
    GLuint m_fragment{};
    GLuint m_program{};
    GLuint m_vao_ref{};
    GLuint m_vbo_ref{};

    // Conversion window (hidden) with the OpenGL context
    GLFWwindow* gltfWindow{nullptr};

    // The GLFW initialization process & the GLAD library are NOT multi-threaded safe :
    // we use a mutex to create the window & initialize the OpenGL resources
    static inline mutex initMutex;

    void checkShaderStatus(const GLuint shader) const;
    void checkProgramStatus() const;
};