#ifndef VRAKTAL_EDITOR_UTILS_YAMLPARSER_H
#define VRAKTAL_EDITOR_UTILS_YAMLPARSER_H
#pragma once

#include <core/manager/ressourceManager.h>
#include <scene/timeline/components/light.h>
#include <scene/timeline/components/mesh.h>
#include <scene/scene.h>

#include <glm/glm.hpp>
#include <fkYAML/node.hpp>

#include <string>
#include <sstream>

namespace utils
{
	namespace YamlParser
	{
		extern RessourceManager* _resManager;

        bool LoadProject(
			const std::filesystem::path& filePath,
			Scene* scene,
			RessourceManager* resManager);

		glm::mat4 ParseMat4(const fkyaml::node& node);
		glm::vec3 ParseVec3(const fkyaml::node& node);

		void ParseEntity(Scene* scene, const fkyaml::node& entityNode);
		void ParseLightComponent(Scene* scene, const fkyaml::node& compNode);
		void ParseMeshComponent(Scene* scene, const fkyaml::node& compNode);

		template<typename KeyframeContainer, typename PropertyReader>
		void ParseKeyframes(const fkyaml::node& keyframesNode, KeyframeContainer& container, PropertyReader&& readPropFn);
	};
}

#endif //VRAKTAL_EDITOR_UTILS_YAMLPARSER_H