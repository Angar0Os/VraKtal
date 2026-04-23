#pragma once
#include <iostream> 
#include "SyncEngine.h"
#include "imgui/imgui.h" 



class TimelineEditor
{
public:

	TimelineEditor();
	~TimelineEditor() = default;

	void getTimelineEditorWindow(AudioManager* audioman);
	void ChangeMainMusic();

	void TimelineWidget(const char* str_id, ImVec2 size);

	void KeyframeExplorer(const char* str_id, ImVec2 size);

	void MusicTrackSlider(const char* str_id, float* current_time, float duration, ImVec2 size);

	void Timelinevizualizer(const char* str_id, ImVec2 size);

	std::string ConvertToTime(float time);

private:
	std::string loadedSong;
	AudioManager* audio;
	float volume;
	float zoomLevel;

};