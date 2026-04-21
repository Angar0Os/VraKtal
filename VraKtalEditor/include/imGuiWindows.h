#pragma once

#include "contentDrawer.h"
#include "TimelineEditor.h"
#include "AudioManager.h"

namespace graphics {
    class Renderer;
}

class ImGuiWindows
{
public:
	ImGuiWindows(graphics::Renderer* _renderer);
	~ImGuiWindows();

    void PrepareImGuiWindows(AudioManager* audio);// en test

private:
	void SetMenuBar();

    void TrackEditorWindow(AudioManager* audio);
	void ContentDrawerWindow();
    void HierarchyWindow();
    void mainWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

	ContentDrawer m_contentDrawer;
    TimelineEditor m_trackEditor;

private :
    graphics::Renderer* m_renderer;
};