#pragma once
#include <iostream> 
#include "SyncEngine.h"
#include "imgui/imgui.h" 

struct {
	
}Cell;


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
	void cell(ImVec2i position);
	void Cursor(const char* str_id, ImVec2 size);


	std::string ConvertToTime(float time);

private:
	std::string loadedSong;
	AudioManager* audio;
	
	float volume;
	
	//cell and visualizer value
	float zoomLevel;
	ImVec2 cellSize;
	std::vector<std::vector<Cell>> cellGrid;

	//cursor value
	ImVec2 cursorPos;
	ImU32 cursorCol;
	float cursorSize;

};