#ifndef EDITOR_IMGUIWINDOWS_H
#define EDITOR_IMGUIWINDOWS_H
#pragma once

#include "contentDrawer.h"
#include "newProjectModal.h"

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
    void LoadProject();

	ContentDrawer m_contentDrawer;
    NewProjectModal m_newProjectModal;

private :
    graphics::Renderer* m_renderer;
};

#endif //EDITOR_IMGUIWINDOWS_H
