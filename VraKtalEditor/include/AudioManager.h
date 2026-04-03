#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <core/window.h>
#include <iostream>
#include <vector>
#include <map>
#include "bass/bass.h"

// display error messages

enum ChannelAttribute{
	BUFFER,
	DOWNMIX,
	FREQ,
	GRANULE,
	MUSIC_AMPLIFY,
	MUSIC_BPM,
	MUSIC_PANSEP,
	MUSIC_PSCALER,
	MUSIC_SPEED,
	MUSIC_VOL_CHAN,
	MUSIC_VOL_GLOBAL,
	MUSIC_VOL_INST,
	NET_RESUME,
	NORAMP,
	PAN,
	PUSH_LIMIT,
	SRC,
	TAIL,
	VOL,
	VOLDSP,
	VOLDSP_PRIORITY
};





inline void Error(const char* es)
{
	std::cout << "%s\n(error code: %d)" << es << BASS_ErrorGetCode();//<< std::cend();
	//sprintf(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
}

struct
{

}KeyFrame;

struct
{
	std::string name;

}Track;

class AudioManager
{
public:
	AudioManager();
	AudioManager(GLFWwindow* win);
	~AudioManager();

	//main music
	void LoadMainMusic(std::string relativeFilePath);
	void getFFT();
	float getTime();
	void PlayMainMusic();
	void PauseMainMusic();

	//sample (for effect and object's sounds)
	void LoadSample(std::string relativeFilePath);
	HCHANNEL playAndGetSample(std::string name);


	void FreeChannel(DWORD handle);
	void PlayChannel(DWORD handle);
	void PauseChannel(DWORD handle);
	void stopChannel();

	void PauseAll();
	void StartAll();

	void changeChannelattribute(DWORD handle, ChannelAttribute attribute, float value);
	

private:
	HSTREAM mainMusic;
	std::map<std::string , HSAMPLE> sampleChannel;
	GLFWwindow* mainWindow;

};


