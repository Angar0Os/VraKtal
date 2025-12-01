#ifndef VRAKTAL_GRAPHICS_RESOURCES_PROJECT_H
#define VRAKTAL_GRAPHICS_RESOURCES_PROJECT_H
#pragma once

#include <graphics/resources/scene.h>
#include <memory>
#include <vector>
#include <string>

namespace graphics::resources
{
	struct TimelineEntry
	{
		std::string sceneName;
		std::string cameraName;
		float tStart = 0.0f;
		float tEnd = 10.0f;
	};

	class Project
	{
	public:
		std::string name;
		std::vector<std::shared_ptr<Scene>> scenes;
		std::vector<TimelineEntry> timeline;
		std::shared_ptr<Scene> activeScene;

		Project() = default;
		explicit Project(const std::string& name) : name(name) {}

		Scene* AddScene(const std::string& sceneName)
		{
			auto scene = std::make_shared<Scene>(sceneName);
			scenes.push_back(scene);

			if (!activeScene)
				activeScene = scene;

			return scene.get();
		}

		Scene* FindSceneByName(const std::string& name)
		{
			auto it = std::find_if(scenes.begin(), scenes.end(),
				[&name](const auto& s) { return s->name == name; });
			return (it != scenes.end()) ? it->get() : nullptr;
		}

		void AddTimelineEntry(const std::string& scene, const std::string& camera,
			float start, float end)
		{
			timeline.push_back({ scene, camera, start, end });
		}

		void Clear()
		{
			scenes.clear();
			timeline.clear();
			activeScene.reset();
		}
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_PROJECT_H