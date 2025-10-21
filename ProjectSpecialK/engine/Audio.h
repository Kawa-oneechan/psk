#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <glm/glm.hpp>
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>
#include <soloud/soloud_wavstream.h>
#include "../Game.h"

//A wrapper around SoLoud, or whatever other audio backend may be used.
class Audio
{
private:
	static SoLoud::Soloud system;
	static std::vector<Audio*> playing;
	SoLoud::Wav sound;
	SoLoud::WavStream stream;
	unsigned int handle{ 0 };
	bool isStream{ false };
#ifdef BECKETT_3DAUDIO
	bool is3D{ false };
#endif
	enum class Status
	{
		Invalid, Stopped, Paused, Playing,
	} status{ Status::Stopped };
	enum class Type
	{
		Music, Sound,
#ifdef BECKETT_MOREVOLUME
		Ambient, Speech
#endif
	} type{ Type::Sound };
	std::string filename;
	std::unique_ptr<char[]> data{ nullptr };
	std::vector<class AudioEventListener*> listeners;
	std::vector<std::tuple<float, std::string>> tags;
	float lastTag{ -1.0f };
	float nextTag{ -1.0f };
	int currentTag{ 0 };

	void update();

public:
	//Is audio enabled in general?
	static bool Enabled;
	//Background music volume
	static float MusicVolume;
#ifndef BECKETT_MOREVOLUME
	//General sound volume
	static float SoundVolume;
#else
	//General sounds -- diegetic and UI.
	static float SoundVolume;
	//Ambient noises -- outside wind, soundscapes.
	static float AmbientVolume;
	//Dialogue sounds -- both vocalizations and beeps.
	static float SpeechVolume;
#endif

	//Initializes SoLoud.
	static void Initialize();
	//Updates SoLoud, then goes through any pending volume changes.
	static void Update();

	//Volume control for this sound.
	float Volume{ 1.0f };

	//Loads a sound file for later use.
	//Depending on the path, its type is set to be music, ambient noise, speeeh,
	//or a general sound. No matter the type, if it's an Ogg Vorbis file it's
	//allowed to loop using the `LOOP_START` tag, specified in samples.
	explicit Audio(const std::string& filename);
	~Audio();
	//Plays the sound. If it's already playing, it won't restart or anything
	//*unless* `force` is true.
#ifndef BECKETT_3DAUDIO
	void Play(bool force = false);
#else
	void Play(bool force = false, bool in3D = true);
#endif
	//Pauses the sound. Calling `Play` afterwards will resume playback.
	void Pause();
	//Stops playing the sound. Calling `Play` afterwards will restart playback.
	void Stop();
	//Mostly for internal use but you never know. Updates both the Volume
	//member and the wrapped SoLoud sound's volume.
	void UpdateVolume();

	//Changes the pitch of the sound. Works best if it's not already playing.
	void SetPitch(float ratio);
	void SetPosition(const glm::vec3& pos);
	static void Audio::SetListenerPosition(const glm::vec3& pos);
	//Sets the sound's position in 2D stereo space where -1.0 is fully
	//to the left and 1.0 is fully to the right.
	void SetPan(float pos);
	void SetLoop(bool loop);

	//Registers an AudioEventListener to receive messages for audio with timed events.
	void RegisterListener(const class AudioEventListener* listener);
	//Unregisters an AudioEventListener.
	void UnregisterLister(const class AudioEventListener* listener);
};

class AudioEventListener
{
public:
	virtual void AudioEvent(float time, const std::string& text) {};
};
