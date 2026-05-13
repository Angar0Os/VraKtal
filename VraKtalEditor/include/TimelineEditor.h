#pragma once
#include <iostream> 
#include "SyncEngine.h"
#include "imgui/imgui.h" 
#include <imgui/imgui_internal.h>

struct Cell {
	ImRect surface;
	std::string strId;
	std::vector<Cell*> subCells;
//keyframe
};


class TimelineEditor
{
public:

	TimelineEditor();
	~TimelineEditor() = default;

	void getTimelineEditorWindow(AudioManager* audioman);
	void ChangeMainMusic();

	void TimelineWidget(const char* str_id, ImVec2 size);

	//timeline component
	void KeyframeExplorer(const char* str_id, ImVec2 size);
	void MusicTrackSlider(const char* str_id, float* current_time, float duration, ImVec2 size);
	void Timelinevisualizer(const char* str_id, ImVec2 size);


	//vizualizer component
	void gridOfCell(const char* str_id, ImVec2 size, int col, int row);
	Cell* CreateCell(const char* str_id, ImVec2 position);
	void Cursor(const char* str_id, ImVec2 size);

	void cellBody(ImRect bb);

	std::string ConvertToTime(float time);

private:
	std::string loadedSong;
	AudioManager* audio;
	
	float volume;
	
	//cell and visualizer value
	float zoomLevel;
	ImVec2 cellSize = ImVec2(100, 20);
	int cellBorderWidth = 1;
	std::vector<std::vector<Cell*>> cellGrid;

	//cursor value
	ImVec2 cursorPos;
	ImU32 cursorCol;
	float cursorSize;


	//sync (seulement pour des test)
	int beatperrow = 4;
	int bpm;

};