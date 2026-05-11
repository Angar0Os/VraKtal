#include "utils/yamlParser.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>

RessourceManager* utils::YamlParser::_resManager = nullptr;

glm::mat4 utils::YamlParser::ParseMat4(const fkyaml::node& node)
{
    glm::mat4 m(1.0f);

    if (node.is_sequence() && node.size() == 4) {
        for (int i = 0; i < 4; ++i) {
            if (node[i].is_sequence() && node[i].size() == 4) {
                m[i][0] = node[i][0].get_value<float>();
                m[i][1] = node[i][1].get_value<float>();
                m[i][2] = node[i][2].get_value<float>();
                m[i][3] = node[i][3].get_value<float>();
            }
        }
    }

    return m;
}

glm::vec3 utils::YamlParser::ParseVec3(const fkyaml::node& node)
{
    glm::vec3 v(0.0f);

    if (node.is_mapping()) {
        if (node.contains("x")) {
            v.x = node["x"].get_value<float>();
        }

        if (node.contains("y")) {
            v.y = node["y"].get_value<float>();
        }

        if (node.contains("z")) {
            v.z = node["z"].get_value<float>();
        }
    }

    return v;
}

template<typename KeyframeContainer, typename PropertyReader>
void utils::YamlParser::ParseKeyframes(const fkyaml::node& keyframesNode, KeyframeContainer& container, PropertyReader&& readPropFn)
{
    if (!keyframesNode.is_sequence()) {
        return;
    }

    for (const auto& kfNode : keyframesNode) {
        typename KeyframeContainer::value_type kf;

        if (kfNode.contains("time")) {
            kf.time = kfNode["time"].get_value<float>();
        }

        if (kfNode.contains("interpolation")) {
            kf.interpolation = static_cast<EInterpolationType>(kfNode["interpolation"].get_value<int>());
        }

        readPropFn(kfNode, kf.property);

        container.push_back(kf);
    }
}

void utils::YamlParser::ParseLightComponent(Scene* scene, const fkyaml::node& compNode)
{
    timeline::Light light;

    if (compNode.contains("bIsActive")) {
        light.bIsActive = compNode["bIsActive"].get_value<bool>();
    }

    auto ReadProp = [](const fkyaml::node& node, timeline::LightProperty& property) {
        if (node.contains("position")) property.position = ParseVec3(node["position"]);
        if (node.contains("direction")) property.direction = ParseVec3(node["direction"]);
        if (node.contains("color")) property.color = ParseVec3(node["color"]);
        if (node.contains("intensity")) property.intensity = node["intensity"].get_value<float>();
        if (node.contains("enabled")) property.enabled = node["enabled"].get_value<bool>();
        if (node.contains("LightType")) property.type = static_cast<timeline::LightType>(node["LightType"].get_value<int>());
        if (node.contains("innerConeAngle")) property.innerConeAngle = node["innerConeAngle"].get_value<float>();
        if (node.contains("outerConeAngle")) property.outerConeAngle = node["outerConeAngle"].get_value<float>();
        if (node.contains("constant")) property.constant = node["constant"].get_value<float>();
        if (node.contains("linear")) property.linear = node["linear"].get_value<float>();
        if (node.contains("quadratic")) property.quadratic = node["quadratic"].get_value<float>();
        if (node.contains("radius")) property.radius = node["radius"].get_value<float>();
        if (node.contains("lightRadius")) property.lightRadius = node["lightRadius"].get_value<float>();
    };

    ReadProp(compNode, light.temp_property);

    if (compNode.contains("keyframes")) {
        ParseKeyframes(compNode["keyframes"], light.keyframes, ReadProp);
    }

    scene->CreateEntity<timeline::Light>(light);
}

void utils::YamlParser::ParseMeshComponent(Scene* scene, const fkyaml::node& compNode)
{
    timeline::MeshInstance mesh;

    if (compNode.contains("bIsActive")) {
        mesh.bIsActive = compNode["bIsActive"].get_value<bool>();
    }

    if (compNode.contains("path") && _resManager) {
        std::string path = compNode["path"].get_value<std::string>();
        mesh.meshID = _resManager->GetRessourceID<graphics::resources::Mesh>(path);
    }

    auto ReadProp = [](const fkyaml::node& node, auto& property) {
        if (node.contains("transform")) {
            property.transform = ParseMat4(node["transform"]);
        }
    };

    ReadProp(compNode, mesh.temp_properties);

    if (compNode.contains("keyframes")) {
        ParseKeyframes(compNode["keyframes"], mesh.keyframes, ReadProp);
    }

    scene->CreateEntity<timeline::MeshInstance>(mesh);
}

void utils::YamlParser::ParseEntity(Scene* scene, const fkyaml::node& entityNode)
{
    std::string name = "Unnamed";
    if (entityNode.contains("name")) {
        name = entityNode["name"].get_value<std::string>();
    }

    if (!entityNode.contains("components")) {
        return;
    }

    for (const auto& compNode : entityNode["components"]) {
        if (!compNode.contains("type")) {
            continue;
        }

        std::string type = compNode["type"].get_value<std::string>();

        if (type == "Light") {
            ParseLightComponent(scene, compNode);
        }
        else if (type == "MeshInstance") {
            ParseMeshComponent(scene, compNode);
        }
    }
}

bool utils::YamlParser::LoadProject(const std::filesystem::path& inputPath, Scene* scene, RessourceManager* resManager)
{
    _resManager = resManager;

    if (!scene) {
        std::cerr << "[YamlParser] Scene is null." << std::endl;
        return false;
    }

    auto startLoad = std::chrono::high_resolution_clock::now();

    try {
        std::ifstream ifs(inputPath);
        if (!ifs.is_open()) {
            std::cerr << "[YamlParser] Cannot open file for reading: " << "\n"; // add path ref

            return false;
        }

        std::vector<fkyaml::node> docs = fkyaml::node::deserialize_docs(ifs);

        if (docs.empty()) {
            return false;
        }

        const fkyaml::node& root = docs[0];

        if (!root.contains("project") || !root["project"].contains("scene")) {
            std::cerr << "[YamlParser] Invalid format: missing project or scene root." << std::endl;

            return false;
        }

        const fkyaml::node& sceneNode = root["project"]["scene"];

        for (const auto& entityNode : sceneNode) {
            ParseEntity(scene, entityNode);
        }

    }
    catch (const fkyaml::exception& e) {
        std::cerr << "[YamlParser] Parse error: " << e.what() << "\n";

        return false;
    }

    auto endLoad = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> loadDuration = endLoad - startLoad;
    std::cout << "Loaded project from YAML in: " << loadDuration.count() << " ms" << std::endl;

    return true;
}