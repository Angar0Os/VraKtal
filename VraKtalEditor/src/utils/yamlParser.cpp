#include "../../header/utils/yamlParser.h"
#include <fstream>
#include <iostream>

namespace utils
{
	YamlParser::YamlParser(const std::string& filePath) : m_filePath(filePath), m_isValid(false)
	{
		LoadFile();
	}

	YamlParser::~YamlParser() {}

	void YamlParser::LoadFile()
	{
		try {
			std::ifstream ifs(m_filePath);
			if (!ifs.is_open()) {
				std::cerr << "Failed to open YAML file: " << m_filePath << std::endl;
				m_isValid = false;
				return;
			}

			std::vector<fkyaml::node> docs = fkyaml::node::deserialize_docs(ifs);
			
			if (docs.empty()) {
				std::cerr << "No YAML documents found in file: " << m_filePath << std::endl;
				m_isValid = false;
				return;
			}
			
			m_root = docs[0];
			m_isValid = true;
		}
		catch (const fkyaml::exception& e) {
			std::cerr << "Failed to load YAML file: " << e.what() << std::endl;
			m_isValid = false;
		}
	}

	std::vector<fkyaml::node> YamlParser::GetObjectsByType(const std::string& type)
	{
		std::vector<fkyaml::node> result;

		if (!m_isValid || !m_root.has_value()) {
			std::cerr << "Cannot get objects: YAML file is invalid" << std::endl;
			return result;
		}

		try {
			const fkyaml::node& root = m_root.value();

			if (!root.contains("project")) {
				std::cerr << "No 'project' key found in YAML" << std::endl;
				return result;
			}

			const fkyaml::node& project = root["project"];

			if (!project.contains("scenes")) {
				std::cerr << "No 'scenes' key found in project" << std::endl;
				return result;
			}

			const fkyaml::node& scenes = project["scenes"];

			for (const auto& scene : scenes) {
				if (!scene.contains("objects")) {
					continue;
				}

				const fkyaml::node& objects = scene["objects"];

				for (const auto& object : objects) {
					if (!object.contains("type")) {
						continue;
					}

					std::string objectType = object["type"].get_value<std::string>();

					if (objectType == type) {
						result.push_back(object);
					}
				}
			}
		}
		catch (const fkyaml::exception& e) {
			std::cerr << "Error getting objects by type: " << e.what() << std::endl;
		}

		return result;
	}

	std::vector<graphics::resources::Light> YamlParser::LoadLights()
	{
		std::vector<graphics::resources::Light> lights;

		std::vector<fkyaml::node> lightNodes = GetObjectsByType("Light");

		for (const auto& node : lightNodes) {
			lights.push_back(ParseLight(node));
		}

		return lights;
	}

	graphics::resources::Light YamlParser::ParseLight(const fkyaml::node& node)
	{
		graphics::resources::Light light;

		// TODO: see if some attribute are mandatory / optional etc

		try {
			if (node.contains("name")) {
				light.name = node["name"].get_value<std::string>();
			}

			if (node.contains("color")) {
				const fkyaml::node& color = node["color"];

				light.color = glm::vec3(
					color["r"].get_value<float>(),
					color["g"].get_value<float>(),
					color["b"].get_value<float>()
				);
			}


			// TODO: refacto bc generic use in much of object types 
			if (node.contains("transform")) {
				const fkyaml::node& transform = node["transform"];

				if (transform.contains("position")) {
					const fkyaml::node& position = transform["position"];
					light.position = glm::vec3(
						position["x"].get_value<float>(),
						position["y"].get_value<float>(),
						position["z"].get_value<float>()
					);
				}
			}

			if (node.contains("intensity")) {
				light.intensity = node["intensity"].get_value<float>();
			}

			if (node.contains("radius")) {
				light.lightRadius = node["radius"].get_value<float>();
			}

			if (node.contains("enabled")) {
				light.enabled = node["enabled"].get_value<bool>();
			}

			// TODO: see for custom light type
			light.type = graphics::resources::LightType::Point;
		}
		catch (const fkyaml::exception& e) {
			std::cerr << "Error parsing light: " << e.what() << std::endl;
		}

		return light;
	}
}