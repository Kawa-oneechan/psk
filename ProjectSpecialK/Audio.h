#pragma once
#include <fmod.hpp>
#include "SpecialK.h"

enum AudioStatus
{
	Invalid, Stopped, Paused, Playing,
};

class Audio
{
private:
	static FMOD::System* Audio::system;
	static std::vector<Audio*> playing;
	FMOD::Sound* theSound;
	FMOD::Channel* theChannel;
	AudioStatus status;
	int type;
	std::string filename;

public:
	static bool Enabled;
	static float MusicVolume;
	static float AmbientVolume;
	static float SoundVolume;
	static float SpeechVolume;
	
	static void Initialize();
	static void Update();

	Audio(std::string filename);
	~Audio();
	void Play(bool force = false);
	void Pause();
	void Stop();
	void UpdateVolume();

	float Volume;
};

