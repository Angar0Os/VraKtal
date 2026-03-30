//#include <iostream> 
//#include <fstream>


#include "AudioManager.h"


AudioManager::AudioManager()
{

	
}

AudioManager::AudioManager(HWND win)
{
	if (BASS_GetVersion() != BASSVERSION)
	{
		//MessageBox(0, "An incorrect version of BASS.DLL was loaded", 0, MB_ICONERROR);
	}
	else
	{
		
		if (!BASS_Init(-1, 44100, 0, win, NULL))
		{
			Error("Can't initialize device");
			//exit;
		}
	}
}                                                                      

AudioManager::~AudioManager()
{
	BASS_Free();
}

void AudioManager::LoadChannel()
{
	//std::ifstream soundFile("assets/sound/Hazbin Hotel - Brighter - (FrançaisFrench).mp3");
	//std::string fileContent(std::istreambuf_iterator<char>(soundFile), std::istreambuf_iterator<char>());

	BASS_StreamCreateFile(0, "assets/sound/Hazbin Hotel - Brighter - (FrançaisFrench).mp3", 0, 0, 0);
	//musicChannel
}

