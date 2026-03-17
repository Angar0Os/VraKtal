#define BASSDEF(f) (winapi *f)
#include "bass/bass.h"

class BassImpl
{
public:
	BassImpl();
	~BassImpl();



private:
	HINSTANCE bass;
	FARPROC BASS_Init;

};

