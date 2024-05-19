#pragma once
#include <fmod.hpp>
#include "SpecialK.h"

typedef enum
{
	Invalid, Stopped, Paused, Playing,
} AudioStatus;

class Audio
{
private:
	static FMOD::System* Audio::system;
	FMOD::Sound* theSound;
	FMOD::Channel* theChannel;
	AudioStatus status;
	std::string filename;
public:
	static void Initialize();
	Audio(std::string filename);
	~Audio();
	void Play();
	void Pause();
	void Stop();
};

