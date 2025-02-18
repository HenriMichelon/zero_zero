/*
 * Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
/*
 * Generate texture atlas from a textual description
 *
 * Textual description format :
 * target_with target_height
 * 1 line per texture : path color_file_name normal_file_name ao_file_name roughness_file_name metal_file_name
 *
 * *_file_name can be "-" if no texture
 *
 * Input textures formats :
 * Diffuse color = RGB or RGBA
 * Normal = RGB OpenGL
 * AO, Roughness, Metalic = Grayscale
 *
 * Output textures formats :
 * Diffuse color = RGBA
 * Normal = RGB OpenGL
 * AO+Roughness+Metalic = R=ambient occlusion G=roughness B=metallic
 *
 */

#include <cxxopts.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "z0/libraries.h"
namespace fs = std::filesystem;

struct Texture {
    int             width{0};
    int             height{0};
    unsigned char*  color{nullptr};
    unsigned char*  normal{nullptr};
    unsigned char*  ao{nullptr};
    unsigned char*  metal{nullptr};
    unsigned char*  rough{nullptr};
};

struct AtlasDescription {
    int             width{0};
    int             height{0};
    vector<Texture> textures;
};

bool verbose{false};

unsigned char* loadTexture(istringstream& iss, Texture& texture, const string& directory, int format) {
    fs::path path(directory);
    string filename;
    if (!(iss >> filename)) {
        cerr << "Failed to parse atlas file" << endl;
        exit(EXIT_FAILURE);
    }
    if (filename == "-") { return nullptr;}
    path /= filename;
    int comp;
    const auto data = stbi_load( path.string().c_str(), &texture.width, &texture.height, &comp, format);
    if (!data) {
        cerr << "Failed to load texture: " << path << endl;
        exit(EXIT_FAILURE);
    }
    if (verbose) { cout << "Loaded texture: " << path << "(" << comp << "->" << format << ")" << endl; }
    return data;
}

AtlasDescription loadAtlasDescription(const string& filepath) {
    AtlasDescription description;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filepath << endl;
        exit(EXIT_FAILURE);
    }

    file >> description.width >> description.height;
    string line;
    while (getline(file, line)) {
        Texture texture;
        istringstream iss(line);

        string directory;
        iss >> directory;
        if (directory.empty()) { continue; }

        texture.color = loadTexture(iss, texture, directory, 4);
        texture.normal = loadTexture(iss, texture, directory, 3);
        texture.ao = loadTexture(iss, texture, directory, 1);
        texture.rough = loadTexture(iss, texture, directory, 1);
        texture.metal = loadTexture(iss, texture, directory, 1);

        description.textures.push_back(texture);
    }

    file.close();
    return description;
}

void generateAtlas(const AtlasDescription& atlas, const string& outputPrefix) {
    auto colorAtlas = vector<unsigned char>(atlas.width * atlas.height * 4);
    auto normalAtlas = vector<unsigned char>(atlas.width * atlas.height * 3);
    auto aoRoughnessMetalnessAtlas = vector<unsigned char>(atlas.width * atlas.height * 3);

    int width = atlas.width;
    int maxHeight = 0;
    int x = 0, y = 0;
    for (const auto& texture : atlas.textures) {
        if (width < texture.width) {
            width = atlas.width;
            if (width < texture.width) {
                cerr << "Output atlas width to small" << endl;
                exit(EXIT_FAILURE);
            }
            y += maxHeight;
            maxHeight = 0;
            x = 0;
        }

        for (int ty = 0; ty < texture.height; ++ty) {
            const int atlasIndex = ((y + ty) * atlas.width + x);
            const int textureIndex = (ty * texture.width);
            memcpy(&colorAtlas[atlasIndex * 4], &texture.color[textureIndex  * 4], texture.width * 4);
            memcpy(&normalAtlas[atlasIndex * 3], &texture.normal[textureIndex  * 3], texture.width * 3);
            for (int tx = 0; tx < texture.width; ++tx) {
                const auto atlasIndexLocal = ((atlasIndex + tx) * 3);
                const auto textureIndexLocal = textureIndex + tx;
                if (texture.ao) {
                    aoRoughnessMetalnessAtlas[atlasIndexLocal] = texture.ao[textureIndexLocal];
                }
                if (texture.rough) {
                    aoRoughnessMetalnessAtlas[atlasIndexLocal + 1] = texture.rough[textureIndexLocal];
                }
                if (texture.metal) {
                    aoRoughnessMetalnessAtlas[atlasIndexLocal + 2] = texture.metal[textureIndexLocal];
                }
            }
        }

        width -= texture.width;
        x += texture.width;
        maxHeight = std::max(maxHeight, texture.height);
    }

    if (verbose) { cout << "Writing color atlas..." << endl; }
    stbi_write_png((outputPrefix + "_color.png").c_str(), atlas.width, atlas.height, 4, colorAtlas.data(), atlas.width * 4);
    if (verbose) { cout << "Writing normal atlas..." << endl; }
    stbi_write_png((outputPrefix + "_normal.png").c_str(), atlas.width, atlas.height, 3, normalAtlas.data(), atlas.width * 3);
    if (verbose) { cout << "Writting AO/Roughness/Metalness atlas..." << endl; }
    stbi_write_png((outputPrefix + "_ao_roughness_metalness.png").c_str(), atlas.width, atlas.height, 3, aoRoughnessMetalnessAtlas.data(), atlas.width * 3);

    for (const auto& texture : atlas.textures) {
        if (texture.color) { stbi_image_free(texture.color); }
        if (texture.normal) { stbi_image_free(texture.normal); }
        if (texture.ao) { stbi_image_free(texture.ao); }
        if (texture.rough) { stbi_image_free(texture.rough); }
        if (texture.metal) { stbi_image_free(texture.metal); }
    }
}

int main(const int argc, char** argv) {
    cxxopts::Options options("genatlas", "Generate a PNG texture atlas from a textual description");
    options.add_options()
        ("v", "Verbose mode")
        ("input", "The textual description file to read", cxxopts::value<string>())
        ("output","The base name of the PNG files to create", cxxopts::value<string>());
    options.parse_positional({"input", "output"});
    const auto result = options.parse(argc, argv);
    if (result.count("input") != 1 || result.count("output") != 1) {
        cerr << "usage: genatlas input.txt output\n";
        return EXIT_FAILURE;
    }

    // -v
    verbose = result.count("v") > 0;

    // positionals args
    const auto &inputFilename  = fs::path(result["input"].as<string>());
    const auto &outputFilename = fs::path(result["output"].as<string>());
    if (!fs::exists(inputFilename)) {
        cerr << "Input file " << inputFilename.string() <<  " not found";
        return EXIT_FAILURE;
    }

    const auto description = loadAtlasDescription(inputFilename.string());
    generateAtlas(description, outputFilename.string());
    if (verbose) { cout << "Atlas generated successfully!" << endl; }

    return 0;
}

