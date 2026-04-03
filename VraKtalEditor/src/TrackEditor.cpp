#pragma once

#include "TrackEditor.h"
#include "imgui/imgui.h" 
#include "AudioManager.h"

TrackEditor::TrackEditor()
{

}

void TrackEditor::getTrackEditorWindow(AudioManager* audio)
{
	ImGui::Begin("Track Editor", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::Button("play"))
	{
		audio->PlayMainMusic();
	}

	if (ImGui::Button("pause"))
	{
		audio->PauseMainMusic();
	}




	//ImTextureID my_tex_id = ;
	//ImGui::ImageButton();

	//if (ImGui::BeginMenuBar())
	//{
	//	ImGui::

	//	ImGui::EndMenuBar();
	//}


	ImGui::End();
}
