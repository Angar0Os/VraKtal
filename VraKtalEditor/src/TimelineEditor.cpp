#pragma once

#include "TimelineEditor.h"
#include "imgui/imgui.h" 
#include "imgui/imgui_Internal.h"
#include "AudioManager.h"
#include "portable-file-dialogs/portable-file-dialogs.h"



TimelineEditor::TimelineEditor()
{
	volume = .5f;
}

void TimelineEditor::getTimelineEditorWindow(AudioManager* audioman)
{
	audio = audioman;
	

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
	//window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin("Track Editor", nullptr, window_flags);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Add Music"))
			{
				ChangeMainMusic();
			}
			ImGui::EndMenu();
		}	
		ImGui::EndMenuBar();
	}
	
	float currentTime = audio->getCurrentTime(loadedSong);
	float totalTime = audio->getMaxTime(loadedSong);
	std::string time = ConvertToTime(currentTime);
	std::string tTime = ConvertToTime(totalTime);
	std::string hourglass = "Time: " + time + " / " + tTime;

	ImGui::Text(hourglass.c_str());

	ImGui::SameLine();
	if (ImGui::Button("play"))
	{
		audio->PlayChannel(loadedSong);
	}

	ImGui::SameLine();
	if (ImGui::Button("pause"))
	{
		audio->PauseChannel(loadedSong);
	}

	ImGui::SameLine();
	if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.0f))
	{
		audio->changeChannelattribute(loadedSong, VOL, volume);
	}

	TimelineWidget("timeline", ImGui::GetContentRegionAvail());

	ImGui::End();
}

void TimelineEditor::ChangeMainMusic()
{
	auto file = pfd::open_file("Select a file...").result()[0];
	if (file != "")
	{
		audio->FreeChannel(loadedSong);
		audio->LoadMainMusic(file);
		audio->changeChannelattribute(file, VOL, volume);
		loadedSong = file;
	}
}

void TimelineEditor::TimelineWidget(const char* str_id, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;


	ImGuiWindowFlags winFlag = ImGuiWindowFlags_HorizontalScrollbar;

	//ImGui::BeginChild("ChildL", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, winFlag);


		ImGui::PushID(str_id);

			const ImGuiID id = window->GetID(str_id);

			ImVec2 pos = window->DC.CursorPos;
			ImRect total_bb(pos, size);  ///ImGui::GetContentRegionAvail()

			ImVec2 keyframeExplorerZone = ImVec2(total_bb.Max.x / 10, total_bb.Max.y);
			//ImVec2 timelineZone = ImVec2(total_bb.Max.x - keyframeExplorerZone.x, total_bb.Max.y);// +(total_bb.Max.y - musicTrackZone.y));
			

			ImDrawList* draw_list = window->DrawList;
			//draw_list->AddRectFilled(total_bb.Min, total_bb.Max, ImGui::GetColorU32(ImGuiCol_Border));

			KeyframeExplorer("keyframeExplorer", keyframeExplorerZone);
			ImGui::SameLine();
			Timelinevizualizer("timeline", ImGui::GetContentRegionAvail());

		ImGui::PopID();


	//ImGui::EndChild();
}

void TimelineEditor::KeyframeExplorer(const char* str_id, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, size);

	ImGui::BeginChild("keyframeExplo", bb.Max, ImGuiChildFlags_ResizeX, ImGuiWindowFlags_MenuBar);
		for (int i = 0; i < 100; i++)
			ImGui::Text("%04d: scrollable region", i);
	ImGui::EndChild();

	//ImGui::ItemSize(bb);
	//if (!ImGui::ItemAdd(bb, id)) return;

	//ImDrawList* draw_list = window->DrawList;
	//draw_list->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Text));
}

void TimelineEditor::MusicTrackSlider(const char* str_id, float* current_time, float duration, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);  //ImGui::GetContentRegionAvail()

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

	// 1. On réserve l'espace dans le layout et on gère l'interaction
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) return;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	// 2. Logique de mise à jour (Drag)
	if (held)
	{
		float mouse_x = ImGui::GetIO().MousePos.x;
		float fraction = ImClamp((mouse_x - bb.Min.x) / bb.Max.x, 0.0f, 1.0f); //GetWidth()
		*current_time = fraction * duration;
		audio->ChangeChannelPosition(loadedSong, *current_time);
	}

	// 3. Rendu visuel avec DrawList
	ImDrawList* draw_list = window->DrawList;
	float grab_x = bb.Min.x + (*current_time / duration) * bb.Max.x;//bb.GetWidth();

		// Fond de la track
	draw_list->AddRectFilled(bb.Min, ImVec2(bb.Max.x + bb.Min.x, bb.Max.y), ImGui::GetColorU32(ImGuiCol_FrameBg), 5.0f);

		// Barre de progression (la partie remplie)
	draw_list->AddRectFilled(bb.Min, ImVec2(grab_x, bb.Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 5.0f);

		// Le curseur (Grab)
	if (hovered || held)
	{
		draw_list->AddCircleFilled(ImVec2(grab_x, bb.GetCenter().y), 8.0f, IM_COL32(255, 255, 255, 255));
	}

	//4. temps de la musique
	std::string time = ConvertToTime(*current_time);

}

void TimelineEditor::Timelinevizualizer(const char* str_id, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, size);

	ImVec2 musicTrackZone = ImVec2(bb.Max.x, pos.y + 20); 
	

	

	float length = audio->getMaxTime(loadedSong);
	float currentTime = audio->getCurrentTime(loadedSong);

	ImGui::BeginGroup();
		MusicTrackSlider("trackSlider", &currentTime, length, musicTrackZone);
		ImVec2 vizualizerZone = ImVec2(bb.Max.x, ImGui::GetContentRegionAvail().y);
		//ImGui::SetCursorScreenPos(ImVec2(cpos.x, musicTrackZone.y));//+ musicTrackZone.y
		ImGui::BeginChild("timelineVizu", vizualizerZone, ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);
			ImVec2 cpos  = ImGui::GetCursorScreenPos();
			ImVec2 ecart = ImVec2(2 * (cpos.x - bb.Min.x), 1.25 * (cpos.y - bb.Min.y ));
			//for (int i = 0; i < 100; i++)
				//ImGui::Text("%04d: scrollable region", i);
				ImDrawList* draw_list = window->DrawList;
				draw_list->AddRectFilled(cpos, ImVec2(bb.Max.x + cpos.x - ecart.x, bb.Max.y + cpos.y - ecart.y), ImGui::GetColorU32(ImGuiCol_Button));
				//draw_list->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y) , ImVec2(bb.Max.x + bb.Min.x, bb.Max.y + bb.Min.y), ImGui::GetColorU32(ImGuiCol_Button));

		ImGui::EndChild();
	ImGui::EndGroup();
}

std::string TimelineEditor::ConvertToTime(float time)
{
	int Seconde = (unsigned int)time; //static_cast<int>(time);
	int hour = time / 60;  // static_cast<int>((currentTime - hour) * 60

	return std::format("{:2}:{:2}", hour, Seconde - (hour * 60));//0:.2s 1:.2s
}