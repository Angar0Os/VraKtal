#pragma once
#include <scene/system/systemBase.h>

class Scene;

namespace graphics {
	class Renderer;
}

class MeshSystem : public SystemBase
{
public:
	MeshSystem(graphics::Renderer* _renderer);
	~MeshSystem();

    void Update(Scene& _scene) override;

private:
    graphics::Renderer* m_renderer;
};