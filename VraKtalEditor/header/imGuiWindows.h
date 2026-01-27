#pragma once

#include "contentDrawer.h"

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
	ImGuiWindows(graphics::Renderer* _renderer);
	~ImGuiWindows();

    void PrepareImGuiWindows();

private:
	void SetMenuBar();

	void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

	ContentDrawer m_contentDrawer;

private :
    graphics::Renderer* m_renderer;
};