#pragma once
#include <fmod.hpp>
#include "SpecialK.h"

class Audio
{
	enum class Status
	{
		Invalid, Stopped, Paused, Playing,
	};

private:
	static FMOD::System* Audio::system;
	static std::vector<Audio*> playing;
	FMOD::Sound* theSound;
	FMOD::Channel* theChannel;
	Status status;
	int type;
	std::string filename;
	std::unique_ptr<char[]> data{ nullptr };

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

