#pragma once
#include <scene/system/systemBase.h>



class Scene;
class RessourceManager;

namespace graphics {
	class Renderer;
}

class MeshSystem : public SystemBase
{
public:
	MeshSystem(graphics::Renderer* _renderer , RessourceManager* _reManager);
	~MeshSystem();

    void Update(Scene& _scene) override;

private:
    graphics::Renderer* m_renderer;
	RessourceManager* m_ressourceManager;
};