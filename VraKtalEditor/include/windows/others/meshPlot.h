#pragma once
#include "../../imGuiWindows.h"

class ImGuiWindows;

namespace graphics::resources
{
    class Mesh;
}

class MeshPlot
{
public:

	MeshPlot(ImGuiWindows* _windowManager);
	~MeshPlot();

    void Draw(graphics::resources::Mesh* _mesh);

private:
    ImGuiWindows* m_windowManager;
};