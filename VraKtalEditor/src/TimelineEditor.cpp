#pragma once

#include "TimelineEditor.h"
#include "imgui/imgui.h" 
#include "imgui/imgui_Internal.h"
#include "AudioManager.h"
#include "portable-file-dialogs/portable-file-dialogs.h"



TimelineEditor::TimelineEditor()
{
	volume = .5f;
	//loadedSong = "assets/sounds/Hazbin Hotel - Brighter.mp3";
	//audio->LoadMainMusic(loadedSong);
	//audio->changeChannelattribute(loadedSong, VOL, volume);	
}

void TimelineEditor::getTimelineEditorWindow(AudioManager* audioman)
{
	audio = audioman;
	


	ImGui::Begin("Track Editor", nullptr, ImGuiWindowFlags_MenuBar);
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
	
	if (ImGui::Button("play"))
	{
		audio->PlayChannel(loadedSong);
	}

	if (ImGui::Button("pause"))
	{
		audio->PauseChannel(loadedSong);
	}

	
	
	if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.0f))
	{
		audio->changeChannelattribute(loadedSong, VOL, volume);
	}

	createTimelineWidget();

	ImGui::End();
}

void TimelineEditor::ChangeMainMusic()
{
	auto file = pfd::open_file("Select a file...").result()[0];
	if (file != "")
	{
		audio->FreeChannel(loadedSong);
		audio->LoadMainMusic(file);//"assets/sounds/sonic-the-hedgehog-gets-bubble-sound-effect.mp3");
		audio->changeChannelattribute(file, VOL, volume);
		loadedSong = file;
	}
}

void TimelineEditor::createTimelineWidget()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;


	ImGuiWindowFlags winFlag = ImGuiWindowFlags_HorizontalScrollbar;

	ImGui::BeginChild("ChildL", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, winFlag);


		ImGui::PushID("test");

			const ImGuiID id = window->GetID("test");

			ImVec2 pos = window->DC.CursorPos;
			ImRect total_bb(pos, ImVec2(ImGui::GetContentRegionAvail().x + pos.x, ImGui::GetContentRegionAvail().y + pos.y));  ///ImGui::GetContentRegionAvail()

			float length = audio->getMaxTime(loadedSong);
			float currentTime = audio->getCurrentTime(loadedSong);
			std::string time = ConvertToTime(currentTime);

			ImGui::Text(time.c_str());
			ImGui::SameLine();
			ImVec2 musicTrackZone = ImVec2(total_bb.Max.x, 15);
			MusicTrackSlider(time.c_str(), &currentTime, length, musicTrackZone);




			ImGui::ItemSize(total_bb);
			if (ImGui::ItemAdd(total_bb, id))
			{
				
				//ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp & ImGuiSliderFlags_WrapAround;
				//if (ImGui::SliderFloat(time.c_str(), &currentTime, 0, length, "", flags))
				//{
				//	audio->ChangeChannelPosition(loadedSong, currentTime);
				//}

				ImDrawList* draw_list = window->DrawList;
				////////draw_list->AddText(ImVec2(pos.x , pos.y), ImGui::GetColorU32(ImGuiCol_Text), "text");
			
				//rectangle music timer


			

				////////rectangle principale
				draw_list->AddRectFilled(total_bb.Min, total_bb.Max, ImGui::GetColorU32(ImGuiCol_Border));


				////////Rectangle line selector
				draw_list->AddRectFilled(ImVec2(total_bb.Min.x + 5, total_bb.Min.y + 5), ImVec2(total_bb.Min.x + 100, total_bb.Max.y), ImGui::GetColorU32(ImGuiCol_Button));




				////////Rectangle timeline viewer
				//////draw_list->AddRectFilled(ImVec2(total_bb.Min.x + 105, total_bb.Min.y + 5), ImVec2(total_bb.Min.x + 695, total_bb.Min.y + 200), ImGui::GetColorU32(ImGuiCol_Text));

			}


		ImGui::PopID();


	ImGui::EndChild();
}

void TimelineEditor::MusicTrackSlider(const char* str_id, float* current_time, float duration, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

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
		float fraction = ImClamp((mouse_x - bb.Min.x) / bb.GetWidth(), 0.0f, 1.0f);
		*current_time = fraction * duration;
		audio->ChangeChannelPosition(loadedSong, *current_time);
	}

	// 3. Rendu visuel avec DrawList
	ImDrawList* draw_list = window->DrawList;
	float grab_x = bb.Min.x + (*current_time / duration) * bb.GetWidth();

		// Fond de la track
	draw_list->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), 5.0f);

		// Barre de progression (la partie remplie)
	draw_list->AddRectFilled(bb.Min, ImVec2(grab_x, bb.Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 5.0f);

		// Le curseur (Grab)
	if (hovered || held)
	{
		draw_list->AddCircleFilled(ImVec2(grab_x, bb.GetCenter().y), 8.0f, IM_COL32(255, 255, 255, 255));
	}
}

std::string TimelineEditor::ConvertToTime(float time)
{
	int Seconde = (unsigned int)time; //static_cast<int>(time);
	int hour = time / 60;  // static_cast<int>((currentTime - hour) * 60

	return std::format("{}:{}", hour, Seconde - (hour * 60));
}