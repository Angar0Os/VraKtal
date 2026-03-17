#include "BassImpl.h"

BassImpl::BassImpl()
{
	BASS_Init = GetProcAddress(bass, "BASS_Init");
	BASS_Init(-1, 44100, 0, HWND, NULL);
}
