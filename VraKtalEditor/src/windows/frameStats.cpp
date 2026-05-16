#include "../../include/windows/frameStats.h"
#include "../../include/imGuiWindows.h"

#include <vraktal.h>
#include <core/time.h>

WindowStat::WindowStat(ImGuiWindows& _imguiWindows) : m_imguiWindow(_imguiWindows) , m_time(m_imguiWindow.GetVraktal().GetTime()) {}

WindowStat::~WindowStat()
{
}

void WindowStat::Draw()
{
	if (m_imguiWindow.BeginWindow("Stats"))
	{
        const core::FrameStats& frame = m_time.GetFrameStats();

        ImGui::Text("FPS: %.1f", frame.fps);
        ImGui::Text("Delta Time: %.3f ms", frame.frameTimeMs);

        ImGui::Separator();

        const auto& profiles = m_time.GetProfiles();

        for (const auto& [categoryName, categoryProfiles] : profiles)
        {
            if (ImGui::TreeNode(categoryName.c_str()))
            {
                for (const auto& [profileName, stats] : categoryProfiles)
                {
                    ImGui::Text(
                        "%s : %.3f ms avg: %.3f ms",
                        profileName.c_str(),
                        stats.lastMs,
                        stats.averageMs
                    );
                }

                ImGui::TreePop();
            }
        }
	}
	m_imguiWindow.EndWindow("Stats");
}
