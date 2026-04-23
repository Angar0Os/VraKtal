#ifndef VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H
#define VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H
#pragma once

#include <core/manager/ressourceManager.h>
#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>
#include <scene/scene.h>

#include <glm/glm.hpp>

#include <string>
#include <sstream>

namespace utils {
    namespace YamlSerializer
    {
        extern RessourceManager* _resManager;

        bool SaveProject(
            const std::filesystem::path& outputPath,
            Scene* scene,
            RessourceManager* resManager);

        std::string Ind(int n);
        std::string FormatVal(float f);
        std::string FormatVal(bool b);
        std::string FormatVal(const std::string& s);
        std::string FormatVal(const glm::vec3& v);
        std::string FormatVal(glm::mat4 m);

        template<typename T>
        void WriteKV(std::ostringstream& ss, int indent, const std::string& key, const T& val);

        template<typename KeyframeContainer, typename PropertyWriter>
        void WriteKeyframes(std::ostringstream& ss, int baseIndent, const KeyframeContainer& keyframes, PropertyWriter&& writePropFn);

        std::string SerializeEntity(const std::vector<ComponentBase*> components, const std::string& name);

        void WriteComponent(std::ostringstream& ss, int indent, const timeline::Light& light);
        void WriteComponent(std::ostringstream& ss, int indent, const timeline::MeshInstance& mesh);
    }
}
#endif //VRAKTAL_EDITOR_UTILS_YAML_SERIALIZER_H