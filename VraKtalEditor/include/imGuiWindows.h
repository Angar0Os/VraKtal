#pragma once

#include <memory>

#include "contentDrawer.h"
#include "command/commandHistory.h"

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
	ImGuiWindows(graphics::Renderer* _renderer);
	~ImGuiWindows();

    void PrepareImGuiWindows();

	command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }

private:
	void SetMenuBar();

	void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

	ContentDrawer m_contentDrawer;

	std::unique_ptr<command::CommandHistory> m_commandHistory;

private :
    graphics::Renderer* m_renderer;
};