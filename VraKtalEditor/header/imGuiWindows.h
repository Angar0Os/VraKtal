#pragma once

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

    void HierarchyWindow();
    void testWindow();
    void EditTransformByIndice(const float* cameraView, const float* cameraProjection, int objIndice);

private :
    graphics::Renderer* m_renderer;
};