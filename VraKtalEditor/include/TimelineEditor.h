#pragma once
#include <iostream> 
#include "SyncEngine.h"



class TimelineEditor
{
public:

	TimelineEditor();
	~TimelineEditor() = default;

	void getTimelineEditorWindow(AudioManager* audioman);
	void ChangeMainMusic();

	void createTimelineWidget();

private:
	std::string loadedSong;
	AudioManager* audio;
	float volume;

};