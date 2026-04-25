#include <scene/system/systems/lightSystem.h>
#include <scene/timeline/components/light.h>
#include <scene/scene.h>

#include <graphics/resources/object/light.h>
#include <graphics/renderer.h>

LightSystem::LightSystem(graphics::Renderer* _renderer) : m_renderer(_renderer)
{
}

LightSystem::~LightSystem()
{
}

void LightSystem::Update(Scene& _scene)
{
    auto& lights = _scene.GetComponentStorage<timeline::Light>();
    for (auto& light : lights.Components())
    {
        graphics::resources::Light alight;
		alight.name = "aled";
		alight.position = light.temp_property.position;
		alight.direction = light.temp_property.direction;
		alight.color = light.temp_property.color;
		alight.intensity = light.temp_property.intensity;
		alight.enabled = light.temp_property.enabled;
		alight.innerConeAngle = light.temp_property.innerConeAngle;
		alight.outerConeAngle = light.temp_property.outerConeAngle;
		alight.constant = light.temp_property.constant;
		alight.linear = light.temp_property.linear;
		alight.quadratic = light.temp_property.quadratic;
		alight.radius = light.temp_property.radius;
		alight.lightRadius = light.temp_property.lightRadius;
		
		switch (light.temp_property.type)
		{
		default:
			break;
		case timeline::LightType::Directional:
			alight.type = graphics::resources::LightType::Directional;
			break;
		case timeline::LightType::Point:
			alight.type = graphics::resources::LightType::Point;
			break;
		case timeline::LightType::Spot:
			alight.type = graphics::resources::LightType::Spot;
			break;
		}
		m_renderer->PushLight(alight);
    }
}