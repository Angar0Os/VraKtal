
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iostream>
#include "bass/bass.h"

// display error messages

inline void Error(const char* es)
{
	std::cout << "%s\n(error code: %d)" << es << BASS_ErrorGetCode();//<< std::cend();
	//sprintf(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
}

struct
{

}KeyFrame;;

struct
{
	std::string name;

}Track;

class AudioManager
{
public:
	AudioManager();
	AudioManager(HWND win);
	~AudioManager();

	void LoadChannel();
	void FreeChannel();

	void PlayChannel();
	void PauseChannel();
	void stopChannel();

	void PauseAll();
	void StartAll();

	void changeChannelVolume();
	

private:
	HMUSIC* musicChannel;
	HSTREAM* streamChannel;
	HSAMPLE* sampleChannel;

};