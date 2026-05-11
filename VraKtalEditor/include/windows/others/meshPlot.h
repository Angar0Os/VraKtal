#pragma once
#include "../../imGuiWindows.h"


class ImGuiWindows;

namespace graphics::assets
{
    struct Mesh;
}

class MeshPlot
{
public:

	MeshPlot(ImGuiWindows* _windowManager);
	~MeshPlot();

    void Draw(graphics::assets::Mesh* _mesh);

private:
    ImGuiWindows* m_windowManager;
};