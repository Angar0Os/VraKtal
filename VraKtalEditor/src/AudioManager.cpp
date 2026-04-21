//#include <iostream> 
//#include <fstream>


#include "AudioManager.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <fstream>



AudioManager::AudioManager()
{

	
}

AudioManager::AudioManager(GLFWwindow* win)
{
	DWORD test = BASS_GetVersion();
	DWORD test2 = BASSVERSION;
	if (HIWORD(BASS_GetVersion()) != BASSVERSION)
	{
		Error("An incorrect version of BASS.DLL was loaded");
	}
	else
	{
		if (!BASS_Init(-1, 44100, 0, glfwGetWin32Window(win), NULL))
		{
			Error("Can't initialize sound device");
			//exit;
		}
		mainWindow = win;
	}
}                                                                      

AudioManager::~AudioManager()
{
	BASS_Free();
}

void AudioManager::LoadMainMusic(std::string relativeFilePath)
{
	if (!streamChannels.contains(relativeFilePath))
	{
		HSTREAM mainMusic = BASS_StreamCreateFile(0, relativeFilePath.c_str(), 0, 0, 0);//BASS_SAMPLE_FLOAT
		streamChannels.insert(std::pair<std::string, HSTREAM>(relativeFilePath, mainMusic));
	}
}

void AudioManager::LoadSample(std::string relativeFilePath)
{
	HSAMPLE sample = BASS_SampleLoad(0, relativeFilePath.c_str(), 0, 0, 0, BASS_SAMPLE_3D);
	sampleChannels.insert(std::pair<std::string, HSAMPLE>(relativeFilePath, sample));
}

HCHANNEL AudioManager::playAndGetSample(std::string name)
{
	HSAMPLE sample = sampleChannels[name];
	HCHANNEL channel = BASS_SampleGetChannel(sample, false);
	this->PlayChannel(channel);
	return channel;
}

void AudioManager::FreeChannel(DWORD handle)
{
	BASS_ChannelFree(handle);
}

void AudioManager::FreeChannel(std::string name)
{
	FreeChannel(streamChannels[name]);
}

void AudioManager::PlayChannel(DWORD handle)
{
	BASS_Start();
	BASS_ChannelPlay(handle, false);
}

void AudioManager::PlayChannel(std::string name)
{
		PlayChannel(streamChannels[name]);	
}

void AudioManager::PauseChannel(DWORD handle)
{
	BASS_ChannelPause(handle);
}

void AudioManager::PauseChannel(std::string name)
{
	PauseChannel(streamChannels[name]);
}

void AudioManager::stopChannel(DWORD handle)
{
	//
}

void AudioManager::PauseAll()
{
	BASS_Pause();
}

void AudioManager::StartAll()
{
	BASS_Start();
}

void AudioManager::changeChannelattribute(DWORD handle, ChannelAttribute attribute, float value)
{
	DWORD attrib = getAttribut(attribute);

	if(attrib != NULL)
		BASS_ChannelSetAttribute(handle, attrib, value);
}

void AudioManager::changeChannelattribute(std::string name, ChannelAttribute attribute, float value)
{
	DWORD attrib = getAttribut(attribute);

	if (attrib != NULL)
		BASS_ChannelSetAttribute(streamChannels[name], attrib, value);
}

DWORD AudioManager::getAttribut(ChannelAttribute attribute)
{
	DWORD attrib;
	switch (attribute) {
	case BUFFER:
		attrib = BASS_ATTRIB_BUFFER;
		break;
	case DOWNMIX:
		attrib = BASS_ATTRIB_DOWNMIX;
		break;
	case FREQ:
		attrib = BASS_ATTRIB_FREQ;
		break;
	case GRANULE:
		attrib = BASS_ATTRIB_GRANULE;
		break;
	case MUSIC_AMPLIFY:
		attrib = BASS_ATTRIB_MUSIC_AMPLIFY;
		break;
	case MUSIC_BPM:
		attrib = BASS_ATTRIB_MUSIC_BPM;
		break;
	case MUSIC_PANSEP:
		attrib = BASS_ATTRIB_MUSIC_PANSEP;
		break;
	case MUSIC_PSCALER:
		attrib = BASS_ATTRIB_MUSIC_PSCALER;
		break;
	case MUSIC_SPEED:
		attrib = BASS_ATTRIB_MUSIC_SPEED;
		break;
	case MUSIC_VOL_CHAN:
		attrib = BASS_ATTRIB_MUSIC_VOL_CHAN;
		break;
	case MUSIC_VOL_GLOBAL:
		attrib = BASS_ATTRIB_MUSIC_VOL_GLOBAL;
		break;
	case MUSIC_VOL_INST:
		attrib = BASS_ATTRIB_MUSIC_VOL_INST;
		break;
	case NET_RESUME:
		attrib = BASS_ATTRIB_NET_RESUME;
		break;
	case NORAMP:
		attrib = BASS_ATTRIB_NORAMP;
		break;
	case PAN:
		attrib = BASS_ATTRIB_PAN;
		break;
	case PUSH_LIMIT:
		attrib = BASS_ATTRIB_PUSH_LIMIT;
		break;
	case SRC:
		attrib = BASS_ATTRIB_SRC;
		break;
	case TAIL:
		attrib = BASS_ATTRIB_TAIL;
		break;
	case VOL:
		attrib = BASS_ATTRIB_VOL;
		break;
	case VOLDSP:
		attrib = BASS_ATTRIB_VOLDSP;
		break;
	case VOLDSP_PRIORITY:
		attrib = BASS_ATTRIB_VOLDSP_PRIORITY;
		break;
	default:
		attrib = NULL;
		break;
	}
	return attrib;;
}
