#pragma once
#include <scene/system/systemBase.h>



class Scene;
class RessourceManager;

namespace graphics {
	class Renderer;
}

class LightSystem : public SystemBase
{
public:
	LightSystem(graphics::Renderer& _renderer);
	~LightSystem();

    void Update(Scene& _scene) override;

private:
	graphics::Renderer& m_renderer;
};