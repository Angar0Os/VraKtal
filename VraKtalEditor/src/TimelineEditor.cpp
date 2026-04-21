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

	ImGui::PushID("test");

	const ImGuiID id = window->GetID("test");;

	ImVec2 label_size = ImGui::CalcTextSize("text", NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImRect total_bb(pos, ImVec2(pos.x + label_size.x, pos.y + label_size.y));

	ImGui::ItemSize(total_bb);
	if (!ImGui::ItemAdd(total_bb, id)) return;
	
	ImDrawList* draw_list = window->DrawList;
	//draw_list->AddText(ImVec2(pos.x , pos.y), ImGui::GetColorU32(ImGuiCol_Text), "text");
	
	draw_list->AddRectFilled(total_bb.Min, total_bb.Max, ImGui::GetColorU32(ImGuiCol_Border));
	draw_list->AddRectFilled(ImVec2(total_bb.Min.x + 5, total_bb.Min.y + 5), ImVec2(total_bb.Min.x + 100, total_bb.Min.y + 200), ImGui::GetColorU32(ImGuiCol_Text));

	draw_list->AddRectFilled(ImVec2(total_bb.Min.x + 105, pos.y + 5), ImVec2(pos.x + 695, pos.y + 200), ImGui::GetColorU32(ImGuiCol_Text));



	
	
	
	ImGui::PopID();
}