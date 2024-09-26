#include "cxxopts.hpp"
import std;
import Z0;

void process(const std::shared_ptr<z0::Node>& node) {
    std::cout << node->toString() << std::endl;
    for (const auto& child : node->getChildren()) {
        process(child);
    }
}

int main(int argc, char** argv) {
    cxxopts::Options options("gltf2json", "Create a JSON scene description from a glTF binary file");
    options.add_options()
        ("glb", "The binary glTF file to read", cxxopts::value<std::string>())
        ("json", "The JSON file to create", cxxopts::value<std::string>());
    options.parse_positional({"glb", "json"});
    const auto result = options.parse(argc, argv);
    if (result.count("glb") != 1 || result.count("json") != 1) {
        std::cerr << "usage: gltf2json <glb> <json>\n";
        return EXIT_FAILURE;
    }
    const auto& glb_file = result["glb"].as<std::string>();
    auto scene = z0::Loader::loadModelFromFile(glb_file, false, true);
    process(scene);
    return EXIT_SUCCESS;
}
