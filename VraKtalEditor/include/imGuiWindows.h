#pragma once

#include "contentDrawer.h"
#include <core/window.h>

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
	ImGuiWindows(graphics::Renderer* _renderer, core::Window* window);
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
    core::Window* m_window;
    graphics::Renderer* m_renderer;
};