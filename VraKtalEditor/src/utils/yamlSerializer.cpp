#include <utils/yamlSerializer.h>

#include <graphics/resources/property/transform.h>

#include <iomanip>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include <fkYAML/node.hpp>
#include <chrono>
#include <iostream>

RessourceManager* utils::YamlSerializer::_resManager = nullptr;

std::string utils::YamlSerializer::Ind(int n)
{
    return std::string(static_cast<size_t>(n) * 2, ' ');
}

std::string utils::YamlSerializer::FormatVal(glm::mat4 m)
{
    std::ostringstream ss;

    ss << "[";
    for (int i = 0; i < 4; ++i)
    {
        ss << "["
            << m[i][0] << ", "
            << m[i][1] << ", "
            << m[i][2] << ", "
            << m[i][3]
            << "]";

        if (i < 3)
        {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str();
}

std::string utils::YamlSerializer::FormatVal(float f)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << f;
    return ss.str();
}

std::string utils::YamlSerializer::FormatVal(bool b)
{
    return b ? "true" : "false";
}

std::string utils::YamlSerializer::FormatVal(const std::string& s)
{
    return "\"" + s + "\"";
}

std::string utils::YamlSerializer::FormatVal(const glm::vec3& v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{ x: " << v.x << ", y: " << v.y << ", z: " << v.z << " }";
    return ss.str();
}

template<typename T>
void utils::YamlSerializer::WriteKV(std::ostringstream& ss, int indent, const std::string& key, const T& val)
{
    if constexpr (std::is_enum_v<T>)
    {
        ss << Ind(indent) << key << ": " << static_cast<std::underlying_type_t<T>>(val) << "\n";
    }
    else
    {
        ss << Ind(indent) << key << ": " << FormatVal(val) << "\n";
    }
}

std::string utils::YamlSerializer::SerializeEntity(const std::vector<ComponentBase*> components, const std::string& name)
{
    std::ostringstream ss;

    // todo refacto
    static const std::unordered_map<std::type_index, std::string> typeNames = {
    { typeid(timeline::Light), "Light" },
    { typeid(timeline::MeshInstance), "MeshInstance" }
    };

    ss << Ind(2) << "- name: \"" << name << "\"\n";
    ss << Ind(3) << "components:\n";

    for (const ComponentBase* comp : components)
    {
        if (!comp) {
            continue;
        }

        auto it = typeNames.find(typeid(*comp));

        if (it == typeNames.end()) {
            continue;
        }

        std::string typeStr = it->second;

        ss << Ind(3) << "- type: " << typeStr << "\n";
        WriteKV(ss, 4, "bIsActive", comp->bIsActive);

        if (auto light = dynamic_cast<const timeline::Light*>(comp))
        {
            WriteComponent(ss, 3, *light);
        }
        else if (auto mesh = dynamic_cast<const timeline::MeshInstance*>(comp))
        {
            WriteComponent(ss, 3, *mesh);
        }
    }

    return ss.str();

}

void utils::YamlSerializer::WriteComponent(std::ostringstream& ss, int indent, const timeline::Light& light)
{
    auto WriteProp = [&](timeline::LightProperty property, int propIndent) {
        WriteKV(ss, propIndent, "position", property.position);
        WriteKV(ss, propIndent, "direction", property.direction);
        WriteKV(ss, propIndent, "color", property.color);
        WriteKV(ss, propIndent, "intensity", property.intensity);
        WriteKV(ss, propIndent, "enabled", property.enabled);
        WriteKV(ss, propIndent, "LightType", property.type);
        WriteKV(ss, propIndent, "innerConeAngle", property.innerConeAngle);
        WriteKV(ss, propIndent, "outerConeAngle", property.outerConeAngle);

        WriteKV(ss, propIndent, "constant", property.constant);
        WriteKV(ss, propIndent, "linear", property.linear);
        WriteKV(ss, propIndent, "quadratic", property.quadratic);
        WriteKV(ss, propIndent, "radius", property.radius);

        WriteKV(ss, propIndent, "lightRadius", property.lightRadius);
    };

    WriteProp(light.temp_property, indent + 1);

    WriteKeyframes(ss, indent + 1, light.keyframes, WriteProp);
}

void utils::YamlSerializer::WriteComponent(std::ostringstream& ss, int indent, const timeline::MeshInstance& mesh)
{
    WriteKV(ss, indent + 1, "path", _resManager->GetRessourcePath<graphics::resources::Mesh>(mesh.meshID));

    auto WriteProp = [&](const auto& property, int propIndent) {
        WriteKV(ss, propIndent, "transform", property.transform);
        };

    WriteProp(mesh.temp_properties, indent + 1);

    WriteKeyframes(ss, indent + 1, mesh.keyframes, WriteProp);
}

template<typename KeyframeContainer, typename PropertyWriter>
void utils::YamlSerializer::WriteKeyframes(std::ostringstream& ss, int baseIndent, const KeyframeContainer& keyframes, PropertyWriter&& writePropFn)
{
    ss << Ind(baseIndent) << "keyframes:" << (keyframes.empty() ? " []\n" : "\n");

    if (!keyframes.empty()) {
        int kfIndent = baseIndent + 1;

        for (const auto& kf : keyframes)
        {
            WriteKV(ss, kfIndent, "- time", kf.time);
            WriteKV(ss, kfIndent + 1, "interpolation", kf.interpolation);

            writePropFn(kf.property, kfIndent + 1);
        }
    }
}

bool utils::YamlSerializer::SaveProject(
    const std::filesystem::path& outputPath,
    Scene* scene,
    RessourceManager* resManager)
{
    _resManager = resManager;

    std::ostringstream ss;

    auto startSave = std::chrono::high_resolution_clock::now();

    ss << "project:\n";
    ss << Ind(1) << "scene:\n";

    auto& var = scene->GetAliveEntities();

    for (size_t i = 0; i < var.size(); i++)
    {
        std::vector<ComponentBase*> components;

        if (scene->GetComponentStorage<timeline::Light>().Has(var[i])) {
            timeline::Light& light = scene->GetComponentStorage<timeline::Light>().Get(var[i]);

            components.push_back(&light);
        }

        if (scene->GetComponentStorage<timeline::MeshInstance>().Has(var[i])) {
            timeline::MeshInstance mesh = scene->GetComponentStorage<timeline::MeshInstance>().Get(var[i]);

            components.push_back(&mesh);
        }

        std::string name = scene->GetComponentStorage<std::string>().Get(var[i]);

        ss << utils::YamlSerializer::SerializeEntity(components, name);
    }

    std::ofstream file(outputPath);
    if (!file.is_open())
    {
        std::cerr << "[YamlSerializer] Cannot open file for writing: " << "\n"; // add path ref
        return false;
    }

    file << ss.str();

    if (!file.good())
    {
        std::cerr << "[YamlSerializer] Write error for: " << "\n";
        return false;
    }

    auto endSave = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> saveDuration = endSave - startSave;
    std::cout << "Saved project to YAML in: " << saveDuration.count() << " ms" << std::endl;

    return true;
}