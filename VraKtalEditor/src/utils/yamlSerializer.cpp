#include <utils/yamlSerializer.h>

#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/resources/property/transform.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.inl>
#include <glm/gtc/quaternion.hpp>

#include <core/gpu/buffer.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>


std::string utils::YamlSerializer::Ind(int n)
{
    return std::string(static_cast<size_t>(n) * 2, ' ');
}

std::string utils::YamlSerializer::Flt(float f)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << f;
    return ss.str();
}

std::string utils::YamlSerializer::Vec3Str(const glm::vec3& v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{ x: " << v.x << ", y: " << v.y << ", z: " << v.z << " }";
    return ss.str();
}

std::string utils::YamlSerializer::SerializeTransform(
    const graphics::resources::property::Transform& t, int indent)
{
    glm::mat4 mat = t.GetMatrix();
    glm::vec3 scale, skew, translation;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, translation, skew, perspective);
    glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(rotation));

    std::ostringstream ss;
    ss << Ind(indent) << "transform:\n";
    ss << Ind(indent + 1) << "position: " << Vec3Str(translation) << "\n";
    ss << Ind(indent + 1) << "rotation: " << Vec3Str(eulerDeg) << "\n";
    ss << Ind(indent + 1) << "scale:    " << Vec3Str(scale) << "\n";
    return ss.str();
}

std::string utils::YamlSerializer::SerializeObject(
    const demo::SceneResource& res, const demo::Scene& scene)
{
    std::ostringstream ss;

    std::string typeStr = "Empty";
    std::visit([&](auto* obj)
        {
            using T = std::decay_t<decltype(*obj)>;
            if constexpr (std::is_same_v<T, graphics::resources::object::Camera>)
                typeStr = "Camera";
            else if constexpr (std::is_same_v<T, graphics::resources::Light>)
                typeStr = "Light";
            else if constexpr (std::is_same_v<T, graphics::resources::Mesh>)
                typeStr = obj ? "StaticMesh" : "Empty";
        }, res.object);

    ss << Ind(3) << "- name: \"" << res.objectName << "\"\n";
    ss << Ind(4) << "type: " << typeStr << "\n";

    if (res.isInTimeline)
        ss << Ind(4) << "inTimeline: true\n";

    ss << SerializeTransform(res.objectTransform, 4);

    std::visit([&](auto* obj)
        {
            using T = std::decay_t<decltype(*obj)>;

            if constexpr (std::is_same_v<T, graphics::resources::object::Camera>)
            {
                if (obj)
                {
                    bool isActive = (scene.GetActiveCamera() == obj);
                    ss << Ind(4) << "camera:\n";
                    ss << Ind(5) << "fov:         " << Flt(obj->fov) << "\n";
                    ss << Ind(5) << "aspectRatio: " << Flt(obj->aspectRatio) << "\n";
                    ss << Ind(5) << "zBounds:     [ " << Flt(obj->zNear) << ", " << Flt(obj->zFar) << " ]\n";
                    ss << Ind(5) << "active:      " << (isActive ? "true" : "false") << "\n";
                }
            }
            else if constexpr (std::is_same_v<T, graphics::resources::Light>)
            {
                if (obj)
                {
                    ss << Ind(4) << "light:\n";
                    ss << Ind(5) << "color:     { r: " << Flt(obj->color.r)
                        << ", g: " << Flt(obj->color.g)
                        << ", b: " << Flt(obj->color.b) << " }\n";
                    ss << Ind(5) << "intensity: " << Flt(obj->intensity) << "\n";
                    ss << Ind(5) << "radius:    " << Flt(obj->radius) << "\n";
                    ss << Ind(5) << "enabled:   " << (obj->enabled ? "true" : "false") << "\n";
                }
            }
            else if constexpr (std::is_same_v<T, graphics::resources::Mesh>)
            {
                if (obj && !obj->sourcePath.empty())
                {
                    ss << Ind(4) << "staticmesh:\n";
                    ss << Ind(5) << "geometry: \"" << obj->sourcePath << "\"\n";
                }
            }
        }, res.object);

    return ss.str();
}


std::string utils::YamlSerializer::SerializeScene(const demo::Scene& scene)
{
    std::ostringstream ss;
    ss << Ind(2) << "- name: \"" << scene.name << "\"\n";
    ss << Ind(3) << "objects:\n";

    for (const auto& res : scene.sceneObjects)
        ss << SerializeObject(res, scene);

    return ss.str();
}

std::string utils::YamlSerializer::SerializeTimeline(const std::vector<demo::Scene>& scenes)
{
    std::ostringstream ss;
    ss << Ind(1) << "timeline:\n";

    for (const auto& scene : scenes)
    {
        std::string camName;
        for (const auto& res : scene.sceneObjects)
        {
            const auto* camPtr = std::get_if<graphics::resources::object::Camera*>(&res.object);
            if (camPtr && *camPtr && scene.GetActiveCamera() == *camPtr)
            {
                camName = res.objectName;
                break;
            }
        }

        ss << Ind(2) << "- scene:  \"" << scene.name << "\"\n";
        if (!camName.empty())
            ss << Ind(3) << "camera: \"" << camName << "\"\n";

        ss << Ind(3) << "tStart: 0.0000\n";
        ss << Ind(3) << "tEnd:   10.0000\n";
    }

    return ss.str();
}

bool utils::YamlSerializer::SaveProject(
    const std::filesystem::path& outputPath,
    const std::vector<demo::Scene>& scenes)
{
    std::ostringstream ss;
    ss << "project:\n";
    ss << Ind(1) << "scenes:\n";

    for (const auto& scene : scenes)
        ss << SerializeScene(scene);

    ss << "\n";
    ss << SerializeTimeline(scenes);

    std::ofstream file(outputPath);
    if (!file.is_open())
    {
        std::cerr << "[YamlSerializer] Cannot open file for writing: " << outputPath << "\n";
        return false;
    }

    file << ss.str();

    if (!file.good())
    {
        std::cerr << "[YamlSerializer] Write error for: " << outputPath << "\n";
        return false;
    }

    std::cout << "[YamlSerializer] Project saved: " << outputPath << "\n";
    return true;
}