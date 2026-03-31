#ifndef VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H
#define VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H
#pragma once

#include <demo/scene.h>
#include <graphics/resources/property/transform.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <filesystem>

namespace utils
{
    class YamlSerializer
    {
    public:
        static bool SaveProject(
            const std::filesystem::path& outputPath,
            const std::vector<demo::Scene>& scenes);

    private:
        static std::string Ind(int n);
        static std::string Flt(float f);
        static std::string Vec3Str(const glm::vec3& v);

        static std::string SerializeTransform(
            const graphics::resources::property::Transform& t, int indent);

        static std::string SerializeObject(
            const demo::SceneResource& res, const demo::Scene& scene);

        static std::string SerializeScene(const demo::Scene& scene);

        static std::string SerializeTimeline(const std::vector<demo::Scene>& scenes);
    };
}

#endif //VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H