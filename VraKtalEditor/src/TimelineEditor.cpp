#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

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
	window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
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

		ImGui::PushID(str_id);

			const ImGuiID id = window->GetID(str_id);

			ImVec2 pos = window->DC.CursorPos;
			ImRect total_bb(pos, pos + size);  


			KeyframeExplorer("keyframeExplorer", ImVec2(size.x / 10, size.y));
			ImGui::SameLine();
			Timelinevisualizer("timeline", ImGui::GetContentRegionAvail());

		ImGui::PopID();
}

void TimelineEditor::KeyframeExplorer(const char* str_id, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, pos + size);

	ImGui::BeginChild("keyframeExplo", size, ImGuiChildFlags_ResizeX, ImGuiWindowFlags_MenuBar);
		for (int i = 0; i < 100; i++)
			ImGui::Text("%04d: scrollable region", i);
	ImGui::EndChild();

}

void TimelineEditor::MusicTrackSlider(const char* str_id, float* current_time, float duration, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);  //ImGui::GetContentRegionAvail()

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, ImVec2(size.x,size.y));

	// 1. On réserve l'espace dans le layout et on gère l'interaction
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) return;

	bool hovered, held;
	ImRect bbBehavior(pos, ImVec2(bb.Max.x + bb.Min.x, bb.Max.y + bb.Min.y));
	bool pressed = ImGui::ButtonBehavior(bbBehavior, id, &hovered, &held);

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
	cursorPos = ImVec2(grab_x, bb.GetCenter().y);
	if (hovered || held)
	{
		cursorCol = IM_COL32(255, 100, 0, 255);
		cursorSize = 2.5;
	}
	else
	{
		cursorSize = 1.5;
		cursorCol = IM_COL32(255, 0, 0, 200);
	}
	draw_list->AddRectFilled(ImVec2(cursorPos.x - cursorSize, bb.Min.y), ImVec2(cursorPos.x + cursorSize, bb.Max.y), cursorCol);
	//4. temps de la musique
	std::string time = ConvertToTime(*current_time);

}

void TimelineEditor::Timelinevisualizer(const char* str_id, ImVec2 size)
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
		ImGui::BeginChild("timelineVizu", vizualizerZone, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
			

			//vizualizerZone = ImVec2(bb.Max.x, ImGui::GetContentRegionAvail().y);
			gridOfCell("gridOfCell", vizualizerZone, 25, 50);

			//ImVec2 cpos  = ImGui::GetCursorScreenPos();
			//ImVec2 ecart = ImVec2((cpos.x - bb.Min.x),(cpos.y - bb.Min.y ));
			//ImGui::SetCursorScreenPos(ImVec2(cpos.x - ecart.x, cpos.y - ecart.y + 25));
			//cpos = ImGui::GetCursorScreenPos();


				Cursor("cursor", ImVec2(bb.Max.x + bb.Min.x, bb.Max.y + bb.Min.y));
		ImGui::EndChild();
	ImGui::EndGroup();
}

void TimelineEditor::gridOfCell(const char* str_id, ImVec2 size, int col, int row)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(pos, pos + ImVec2(cellSize.x * col, cellSize.y * row));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) return;
	std::vector<Cell*> cellRow;

	for (int i = 0; i < col; i++)
	{
		cellRow.clear();
		ImGui::SameLine();
		for (int j = 0; j < row; j++)
		{	
			std::string gridId = "Cell_" + std::to_string(i) + "_" + std::to_string(j);

			//ImGui::PushID(i + j);
			cellRow.push_back(CreateCell(gridId.c_str(), bb.Min + ImVec2(cellSize.x * i, cellSize.y * j)));
			//ImGui::PopID();
		}
		cellGrid.push_back(cellRow);
	}

}

Cell* TimelineEditor::CreateCell(const char* str_id, ImVec2 position)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return nullptr;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	

	ImVec2 spos = ImGui::GetCursorScreenPos();
	ImRect sbb(position, position + cellSize);

	Cell* result = new Cell;
	result->surface = sbb;

	cellBody(sbb);

	ImDrawList* draw_list = window->DrawList;
	ImVec2 minVecSubCell;
	ImVec2 maxVecsubCell;

	for (int i = 0; i < beatperrow; i++)
	{
		std::string subId = "sub" + std::string(str_id) + "_" + std::to_string(i);
		const ImGuiID id = window->GetID(subId.c_str());

		minVecSubCell = ImVec2(sbb.Min.x + (( (sbb.Max.x - sbb.Min.x) / beatperrow) * (i)), sbb.Min.y);
		maxVecsubCell = ImVec2(sbb.Min.x + (( (sbb.Max.x - sbb.Min.x) / beatperrow) * (i + 1)), sbb.Max.y);
		ImRect rectSub(minVecSubCell, maxVecsubCell);//sbb.Min

		

		ImGui::PushID(id);
		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(rectSub, id, &hovered, &held);
		if (held)
		{
			
		}
		else if (hovered)
		{
			draw_list->AddRectFilled(rectSub.Min, rectSub.Max, ImGui::GetColorU32(ImGuiCol_Border));
			ImGui::SetItemTooltip(subId.c_str());
		}
		else
		{

		}
		draw_list->AddLine(rectSub.Min, ImVec2(rectSub.Min.x, rectSub.Max.y), ImGui::GetColorU32(ImGuiCol_Border));
		draw_list->AddLine(rectSub.Max, ImVec2(rectSub.Max.x, rectSub.Min.y), ImGui::GetColorU32(ImGuiCol_Border));
		ImGui::PopID();
	}

	return result;
}

void TimelineEditor::Cursor(const char* str_id, ImVec2 size)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(str_id);

	ImVec2 pos = window->DC.CursorPos;
	ImRect bb(cursorPos, size);

	ImDrawList* draw_list = window->DrawList;
	draw_list->AddRectFilled(ImVec2(cursorPos.x - cursorSize * 2, cursorPos.y), ImVec2(cursorPos.x + cursorSize * 2, size.y), cursorCol);
}

void TimelineEditor::cellBody(ImRect bb)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) ;

	ImDrawList* draw_list = window->DrawList;
	draw_list->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), ImGui::GetColorU32(ImGuiCol_Text));
	draw_list->AddLine(ImVec2(bb.Min.x + cellBorderWidth, bb.Min.y) , ImVec2(bb.Min.x + cellBorderWidth, bb.Max.y) , ImGui::GetColorU32(ImGuiCol_Text));
	draw_list->AddLine(ImVec2(bb.Max.x + cellBorderWidth, bb.Max.y) , ImVec2(bb.Max.x + cellBorderWidth, bb.Min.y), ImGui::GetColorU32(ImGuiCol_Text));
	draw_list->AddLine(bb.Max, ImVec2(bb.Min.x, bb.Max.y), ImGui::GetColorU32(ImGuiCol_Text));
}

std::string TimelineEditor::ConvertToTime(float time)
{
	if (time < 0)
	{
		return "00:00";
	}
	else
	{
		int minute = time / 60;  // static_cast<int>((currentTime - hour) * 60
		int Seconde = (unsigned int)time - (minute * 60); //static_cast<int>(time);
		std::string strSeconde; //= std::format("{:2}:{:2}", hour, Seconde - (hour * 60));//0:.2s 1:.2s;
		std::string strMinute;


		if (Seconde < 10)
			strSeconde = std::format("0{}", Seconde);
		else if (Seconde == 0)
			strSeconde = std::format("00");
		else
			strSeconde = std::format("{:2}", Seconde);


		if (minute)
			strMinute = std::format("0{}", minute);
		else if (minute == 0)
			strMinute = std::format("00");
		else
			strMinute = std::format("{:2}", minute);

		return strMinute + ":" + strSeconde;// std::format("{}:{}", strMinute, strSeconde);//0:.2s 1:.2s
	}
	
}