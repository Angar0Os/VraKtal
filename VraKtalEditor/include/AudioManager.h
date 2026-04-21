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
	//void getFFT();
	float getCurrentTime(std::string name);
	float getMaxTime(std::string name);


	//sample (for effect and object's sounds)
	void LoadSample(std::string relativeFilePath);
	HCHANNEL playAndGetSample(std::string name);



	// Sound control
	void PlayChannel(DWORD handle);
	void PlayChannel(std::string name);

	void PauseChannel(DWORD handle);
	void PauseChannel(std::string name);

	void FreeChannel(DWORD handle);
	void FreeChannel(std::string name);

	void StopChannel(DWORD handle);
	void StopChannel(std::string name);

	void PauseAll();
	void StartAll();

	void ChangeChannelPosition(DWORD handle, float position);
	void ChangeChannelPosition(std::string name, float position);

	void changeChannelattribute(DWORD handle, ChannelAttribute attribute, float value);
	void changeChannelattribute(std::string name, ChannelAttribute attribute, float value);
	

private:
	DWORD getAttribut(ChannelAttribute attribute);

	std::map<std::string, HSTREAM> streamChannels;;
	std::map<std::string , HSAMPLE> sampleChannels;
	GLFWwindow* mainWindow;

	

};


