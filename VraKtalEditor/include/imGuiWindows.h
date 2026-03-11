#pragma once

#include <memory>

#include "contentDrawer.h"
#include "command/commandHistory.h"

namespace core::gpu {
	class ImguiContext;
}

class ImGuiWindows
{
public:
	ImGuiWindows(core::gpu::ImguiContext* _imGuiContext);
	~ImGuiWindows();

    void PrepareImGuiWindows();

	command::CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }

private:
	void SetMenuBar();

	void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
	void Viewport();

    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

	ContentDrawer m_contentDrawer;

	std::unique_ptr<command::CommandHistory> m_commandHistory;

private :
	core::gpu::ImguiContext* m_imGuiContext;
};