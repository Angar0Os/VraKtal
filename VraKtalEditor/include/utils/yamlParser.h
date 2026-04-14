#ifndef EDITOR_UTILS_YAMLPARSER_H
#define EDITOR_UTILS_YAMLPARSER_H
#pragma once

#include <graphics/resources/object/light.h>
#include <fkYAML/node.hpp>
#include <string>
#include <vector>
#include <optional>

namespace utils
{
	class YamlParser
	{
	private:
		std::string m_filePath;
		bool m_isValid;
		std::optional<fkyaml::node> m_root;

		void LoadFile();

		graphics::resources::Light ParseLight(const fkyaml::node& node);

	public:
		YamlParser(const std::string& filePath);
		~YamlParser();

		std::vector<fkyaml::node> GetObjectsByType(const std::string& type);

		std::vector < graphics::resources::Light> LoadLights();

		bool IsValid() const { return m_isValid; }
	};
}

#endif //EDITOR_UTILS_YAMLPARSER_H