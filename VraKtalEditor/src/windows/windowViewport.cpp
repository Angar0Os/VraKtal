#include "../../include/windows/windowViewport.h"
#include "../../include/imGuiWindows.h"
#include "../../include/windows/others/imGuizmoHelper.h"

#include <core/gpu/imguiContext.h>
#include <core/gpu/buffer.h>

#include <imgui/imgui.h>

#include <algorithm>
#include <GLFW/glfw3.h>


WindowViewport::WindowViewport(ImGuiWindows& _imguiWindows) : m_imguiWindows(&_imguiWindows){}

WindowViewport::~WindowViewport()
{
}

void WindowViewport::Draw()
{
	if (m_imguiWindows->BeginWindow("Viewport", true))
	{
		ImVec2 avail = ImGui::GetContentRegionAvail();
		uint32_t width = std::max(1u, static_cast<uint32_t>(avail.x));
		uint32_t height = std::max(1u, static_cast<uint32_t>(avail.y));

		m_imguiWindows->GetContext()->DrawViewportComponent(width, height);

		if (m_imguiWindows->GetContext()->GetViewportState()->hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			float centerX = (min.x + max.x) * 0.5f;
			float centerY = (min.y + max.y) * 0.5f;
			glfwSetCursorPos(m_imguiWindows->GetWindow()->GlfwHandle(), centerX, centerY);

			glfwSetInputMode(m_imguiWindows->GetWindow()->GlfwHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			if (glfwRawMouseMotionSupported())
				glfwSetInputMode(m_imguiWindows->GetWindow()->GlfwHandle(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			if (glfwRawMouseMotionSupported())
				glfwSetInputMode(m_imguiWindows->GetWindow()->GlfwHandle(), GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			glfwSetInputMode(m_imguiWindows->GetWindow()->GlfwHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		m_imguiWindows->GetImGuizmoHelper()->DrawGuizmo();
	}
	m_imguiWindows->EndWindow("Viewport");
}
