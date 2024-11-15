/*
 * Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
// Derived from
// https://github.com/GPUOpen-Tools/compressonator/blob/master/applications/_plugins/cmp_gpu/gpuhw/compute_gpuhw.cpp
//=============================================================================
// Copyright (c) 2020-2024    Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//==============================================================================
module;
#include "glad/glad.h"
#include <GLFW/glfw3.h>
import std;
using namespace std;

module converter;

const string WIN_SHADER_VERT =
R"(#version 330 core
layout(location = 0) in vec2 position;

out vec2 v_texCoords;

void main() {
    v_texCoords = (position + 1.0) * 0.5;
    gl_Position = vec4(position, 1.0, 1.0);
}
)";

const string SHADER_FRAG =
R"(#version 330 core
in vec2 v_texCoords;

out vec4 fragmentColor;

uniform sampler2D tex;

void main() {
    fragmentColor = texture(tex, v_texCoords);
}
)";

constexpr float FULL_SCREEN_QUAD[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

// void APIENTRY OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
//                                          GLenum severity, GLsizei length,
//                                          const GLchar* message,
//                                          const void* userParam) {
//     std::cerr << "OpenGL Debug Message:\n";
//     std::cerr << "Source: " << source << "\n";
//     std::cerr << "Type: " << type << "\n";
//     std::cerr << "ID: " << id << "\n";
//     std::cerr << "Severity: " << severity << "\n";
//     std::cerr << "Message: " << message << "\n";
//     std::cerr << "------------------------\n";
// }

Converter::Converter() {
    {
        // Initialize the GLFW window & OpenGL context
        // Protected by a mutex to avoid errors with GLFW & GLAD
        // (there is a lot of static variables in GLAD which makes it non thread-safe)
        lock_guard lock(initMutex);
        if (!glfwInit()) {
            throw "Failed to initialize GLFW3";
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        gltfWindow = glfwCreateWindow(512, 512, "Texture Compression", nullptr, nullptr);
        if (!gltfWindow) {
            glfwTerminate();
            throw "Failed to create GLFW3 window";
        }
        glfwMakeContextCurrent(gltfWindow);
        glfwSwapInterval(1);
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_RENDERBUFFER);
    glEnable(GL_FRAMEBUFFER);
    // glEnable(GL_DEBUG_OUTPUT);
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr);

    try {
        // Initialize window shaders & buffers
        const auto vertexSrc = WIN_SHADER_VERT.c_str();
        w_vertex             = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(w_vertex, 1, &vertexSrc, nullptr);
        glCompileShader(w_vertex);
        checkShaderStatus(w_vertex);

        const auto fragmentSrc = SHADER_FRAG.c_str();
        w_fragment             = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(w_fragment, 1, &fragmentSrc, nullptr);
        glCompileShader(w_fragment);
        checkShaderStatus(w_fragment);

        w_program = glCreateProgram();
        glAttachShader(w_program, w_vertex);
        glAttachShader(w_program, w_fragment);
        glLinkProgram(w_program);
        checkProgramStatus();

        glGenVertexArrays(1, &w_vao_ref);
        glGenVertexArrays(1, &w_vbo_ref);
        glBindVertexArray(w_vao_ref);
        glBindBuffer(GL_ARRAY_BUFFER, w_vbo_ref);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(FULL_SCREEN_QUAD),
                     reinterpret_cast<const uint8_t *>(FULL_SCREEN_QUAD),
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

        // Initialize conversion shaders & buffers
        m_vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vertex, 1, &vertexSrc, nullptr);
        glCompileShader(m_vertex);
        checkShaderStatus(m_vertex);

        m_fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_fragment, 1, &fragmentSrc, nullptr);
        glCompileShader(m_fragment);
        checkShaderStatus(m_fragment);

        m_program = glCreateProgram();
        glAttachShader(m_program, m_vertex);
        glAttachShader(m_program, m_fragment);
        glLinkProgram(m_program);
        checkProgramStatus();

        glGenVertexArrays(1, &m_vao_ref);
        glGenBuffers(1, &m_vbo_ref);
    } catch (...) {
        throw "Failed to create OpenGL shaders";
    }
}

bool Converter::convert(const MipLevel& inMipLevel,
                        MipLevel& outMipLevel,
                        const uint32_t srcChannels,
                        const string& dstFormat) const {
    if (!formats.contains(dstFormat)) {
        cerr << "Unsupported format " << dstFormat << endl;
        return false;
    }
    if (srcChannels != 3 && srcChannels != 4) {
        cerr << "Image must be RGB or RGBA\n";
        return false;
    }
    const auto srcWidth = inMipLevel.width;
    const auto srcHeight = inMipLevel.height;
    const auto srcData = inMipLevel.data.get();

    glUniform1i(glGetUniformLocation(m_program, "tex"),0);
    glBindVertexArray(m_vao_ref);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_ref);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FULL_SCREEN_QUAD), reinterpret_cast<const uint8_t*>(FULL_SCREEN_QUAD), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    int width, height;
    glfwGetFramebufferSize(gltfWindow, &width, &height);

    // Create the destination texture
    GLuint destination;
    glGenTextures(1, &destination);
    glBindTexture(GL_TEXTURE_2D, destination);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Create texture from the source image
    GLuint source;
    glGenTextures(1, &source);
    glBindTexture(GL_TEXTURE_2D, source);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    if (srcChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcWidth, srcHeight, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, srcData->data());
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, srcWidth, srcHeight, 0,
            GL_RGB, GL_UNSIGNED_BYTE, srcData->data());
    }

    // Create framebuffer with renderbuffer
    GLuint fbo, fboColor, fboDepth;

    glGenRenderbuffers(1, &fboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, fboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, srcWidth, srcHeight);

    glGenTextures(1, &fboColor);
    glBindTexture(GL_TEXTURE_2D, fboColor);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDER, fboDepth);

    glBindTexture(GL_TEXTURE_2D, fboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, srcWidth, srcHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);

    glViewport(0, 0, srcWidth, srcHeight);
    glBindVertexArray(m_vao_ref);
    glBindTexture(GL_TEXTURE_2D, source);
    glUseProgram(w_program);
    glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

    outMipLevel.width = srcWidth;
    outMipLevel.height = srcHeight;
    outMipLevel.data = make_shared<vector<uint8_t>>(static_cast<vector<uint8_t>::size_type>(srcWidth * srcHeight * srcChannels));

    // Set the mipmap levels for the fbo color texture
    glBindTexture(GL_TEXTURE_2D, fboColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Get the compressed image
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);
    glBindTexture(GL_TEXTURE_2D, destination);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, formats.at(dstFormat), 0, 0, srcWidth, srcHeight, 0);
    GLint compressedSize = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, outMipLevel.data->data());

    // Cleanup
    glDeleteTextures(1, &source);
    glDeleteTextures(1, &fboColor);
    glDeleteRenderbuffers(1, &fboDepth);
    glDeleteFramebuffers(1, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, w_vao_ref);

    // Bind the results
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, destination);

    // shader
    glUseProgram(w_program);
    glUniform1i(glGetUniformLocation(w_program, "tex"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

    glfwSwapBuffers(gltfWindow);
    glfwPollEvents();
    return true;
}


void Converter::checkShaderStatus(const GLuint shader) const {
    int  success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw "Failed to compile shader error: " + std::string(infoLog);
    };
}

void Converter::checkProgramStatus() const {
    int  success;
    glGetProgramiv(w_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(w_program, 512, nullptr, infoLog);
        throw "Failed to link shader error: " + std::string(infoLog);
    };
}

Converter::~Converter() {
    if (m_vbo_ref) {
        glDeleteVertexArrays(1, &m_vbo_ref);
    }
    if (m_vao_ref) {
        glDeleteVertexArrays(1, &m_vao_ref);
    }
    if (m_program) {
        glDeleteProgram(m_program);
    }
    if (m_vertex) {
        glDeleteShader(m_vertex);
    }
    if (m_fragment) {
        glDeleteShader(m_fragment);
    }
    if (w_program) {
        glDeleteProgram(w_program);
    }
    if (w_vertex) {
        glDeleteShader(w_vertex);
    }
    if (w_fragment) {
        glDeleteShader(w_fragment);
    }
    if (gltfWindow) {
        glfwDestroyWindow(gltfWindow);
        gltfWindow = nullptr;
    }
}


/*bool Converter::getMipMaps(vector<MipLevel>& inMipLevels,
                   const uint32_t srcWidth,
                   const uint32_t srcHeight,
                   const uint32_t srcChannels,
                   const uint8_t* srcData) const {
    GLuint source;
    glGenTextures(1, &source);
    glBindTexture(GL_TEXTURE_2D, source);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glGenerateMipmap(GL_TEXTURE_2D);
    if (srcChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcWidth, srcHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcData);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, srcWidth, srcHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, srcData);
    }

    int level = 0;
    auto width = srcWidth;
    auto height = srcHeight;
    while (width > 1 || height > 1) {
        const auto dataSize = width * height * srcChannels;
        const auto data = new uint8_t[dataSize];
        glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, data);
        inMipLevels.push_back({width,height,dataSize, data});
        level++;
        width = width >> level;
        height = height >> level;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &source);
    return true;
}*/