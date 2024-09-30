#include "cxxopts.hpp"
import std;
import Z0;

const auto types = std::map<z0::Node::Type, std::string>{
    { z0::Node::NODE, "node"},
    { z0::Node::MESH_INSTANCE, "mesh"}
};

void process(const std::string &              scene_id,
             const std::string &              parent,
             const std::shared_ptr<z0::Node> &node,
             std::ofstream &                  out,
             const int                        level) {
    if (!types.contains(node->getType())) {
        std::cerr << "unknown node type : " << node->getType() << std::endl;
        return;
    }
    out << ",\n";
    auto tabs = std::string(level * 2, ' ');
    out << tabs << "{\n";
    tabs = std::string(level * 2 + 2, ' ');
    out << "      \"id\": \"" << node->toString() << "\",\n";
    out << "      \"type\": \"" << types.at(node->getType()) << "\",\n";
    out << "      \"resource\": \"" << scene_id << "\",\n";
    out << "      \"path\": \"" << parent << (parent.empty() ? "" : "/") << node->toString() << "\"\n";
    tabs = std::string(level * 2, ' ');
    out << tabs << "}";
    auto parent_path = std::string{};
    if (parent.empty()) {
        parent_path = node->toString();
    } else {
        parent_path = parent + "/" + node->toString();
    }
    for (const auto &child : node->getChildren()) {
        process(scene_id, parent_path, child, out, level + 1);
    }
}

int main(const int argc, char **argv) {
    cxxopts::Options options("gltf2json", "Create a JSON scene description from a glTF binary file");
    options.add_options()("glb", "The binary glTF file to read", cxxopts::value<std::string>())(
            "json",
            "The JSON file to create",
            cxxopts::value<std::string>());
    options.parse_positional({"glb", "json"});
    const auto result = options.parse(argc, argv);
    if (result.count("glb") != 1 || result.count("json") != 1) {
        std::cerr << "usage: gltf2json <glb> <json>\n";
        return EXIT_FAILURE;
    }
    const auto &glb_filename  = result["glb"].as<std::string>();
    const auto &json_filename = result["json"].as<std::string>();

    const auto     scene    = z0::Loader::loadModelFromFile(glb_filename, false, true);
    auto           out      = std::ofstream(json_filename, std::ios::out);
    constexpr auto scene_id = "sceneResources";
    out << "{\n  \"nodes\": [\n";
    out << "    {\n";
    out << "      \"id\": \"" << scene_id << "\",\n";
    out << "      \"type\": \"model\",\n";
    out << "      \"resource\": \"" << glb_filename << "\"\n";
    out << "    }";
    for (const auto &child : scene->getChildren()) {
        process(scene_id, "", child, out, 2);
    }
    out << "\n  ]\n}";
    out.close();
    return EXIT_SUCCESS;
}
