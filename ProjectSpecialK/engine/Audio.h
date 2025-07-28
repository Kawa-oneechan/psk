#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fmodex/fmod.hpp>
#include <glm/glm.hpp>

class Audio
{
private:
	static FMOD::System* Audio::system;
	static std::vector<Audio*> playing;
	FMOD::Sound* theSound{ nullptr };
	FMOD::Channel* theChannel{ nullptr };
	enum class Status
	{
		Invalid, Stopped, Paused, Playing,
	} status{ Status::Stopped };
	enum class Type
	{
		Music, Ambient, Sound, Speech
	} type{ Type::Sound };
	std::string filename;
	std::unique_ptr<char[]> data{ nullptr };
	float frequency{ 0 };

public:
	//Is audio enabled in general?
	static bool Enabled;
	//Background music volume -- outside hourly tracks, interiors, events.
	static float MusicVolume;
	//Ambient noises -- outside wind, soundscapes.
	static float AmbientVolume;
	//General sounds -- diegetic and UI.
	static float SoundVolume;
	//Dialogue sounds -- both vocalizations and beeps.
	static float SpeechVolume;

	static void Initialize();
	static void Update();

	float Volume{ 1.0f };

	Audio(std::string filename);
	~Audio();
	void Play(bool force = false);
	void Pause();
	void Stop();
	void UpdateVolume();

	void SetPitch(float ratio);
	void SetPosition(glm::vec3 pos);
	void SetPan(float pos);
};

extern std::map<std::string, std::map<std::string, std::shared_ptr<Audio>>> generalSounds;
