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

//TODO: replace SoundTypes and their respective volumes with Buses.
enum class SoundType
{
	Music, Sound,
#ifdef BECKETT_MOREVOLUME
	Ambient, Speech
#endif
};

class AudioSoundBase
{
protected:
	unsigned int handle{ 0 };
	SoundType type{ SoundType::Sound };
	std::string filename;
	std::unique_ptr<char[]> data{ nullptr };

	enum class Status
	{
		Invalid, Stopped, Paused, Playing,
	} status{ Status::Stopped };

	std::vector<class AudioEventListener*> listeners;

	std::vector<std::tuple<float, std::string>> tags;
	float lastTag{ -1.0f };
	float nextTag{ -1.0f };
	int currentTag{ 0 };
	float panPot{ 0.0f };
	float volume{ 1.0f };

public:
#ifdef BECKETT_3DAUDIO
	bool is3D{ false };
#endif
	//Volume control for this sound.
	float Volume{ 1.0f };

	virtual void update() {};
#ifndef BECKETT_3DAUDIO
	virtual void Play(bool force = false) {};
#else
	virtual void Play(bool force = false, bool in3D = true) {};
#endif
	bool IsPlaying() const { return status == Status::Playing; }
	//Pauses the sound. Calling `Play` afterwards will resume playback.
	void Pause();
	//Stops playing the sound. Calling `Play` afterwards will restart playback.
	void Stop();
	//Mostly for internal use but you never know. Updates both the Volume
	//member and the wrapped SoLoud sound's volume.
	void UpdateVolume();
	//Changes the pitch of the sound. Works best if it's not already playing.
	void SetPitch(float ratio);
	//Sets the sound's position in 3D space.
	void SetPosition(const glm::vec3& pos);
	//Sets the sound's position in 2D stereo space where -1.0 is fully
	//to the left and 1.0 is fully to the right.
	void SetPan(float pos);
	virtual void SetLoop(bool loop) {};

	//Registers an AudioEventListener to receive messages for audio with timed events.
	void RegisterListener(const class AudioEventListener* listener);
	//Unregisters an AudioEventListener.
	void UnregisterListener(const class AudioEventListener* listener);
};

//A wrapper around SoLoud, or whatever other audio backend may be used.
class Audio
{
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

	//Sets the listener's position in 3D space.
	static void Audio::SetListenerPosition(const glm::vec3& pos);
};

class Sound : public AudioSoundBase
{
	SoLoud::Wav sound;
	void update() override;

public:
	//Loads a sound file for later use.
	//Depending on the path, its type is set to be music, ambient noise, speeeh,
	//or a general sound. No matter the type, if it's an Ogg Vorbis file it's
	//allowed to loop using the `LOOP_START` tag, specified in samples.
	explicit Sound(const std::string& filename, SoundType type = SoundType::Sound);
	~Sound();

	//Plays the sound. If it's already playing, it won't restart or anything
	//*unless* `force` is true.
#ifndef BECKETT_3DAUDIO
	void Play(bool force = false) override;
#else
	void Play(bool force = false, bool in3D = true) override;
#endif
	//Sets if the sound should repeat automatically.
	void SetLoop(bool loop) override;

};

class Stream : public AudioSoundBase
{
	SoLoud::WavStream stream;
	void update() override;

public:
	std::string Name, Artist;

	//Loads a sound file for later use.
	//Depending on the path, its type is set to be music, ambient noise, speeeh,
	//or a general sound. No matter the type, if it's an Ogg Vorbis file it's
	//allowed to loop using the `LOOP_START` tag, specified in samples.
	explicit Stream(const std::string& filename, SoundType type = SoundType::Music);
	~Stream();

	//Plays the sound. If it's already playing, it won't restart or anything
	//*unless* `force` is true.
#ifndef BECKETT_3DAUDIO
	void Play(bool force = false) override;
#else
	void Play(bool force = false, bool in3D = true) override;
#endif
	//Sets if the sound should repeat automatically.
	void SetLoop(bool loop) override;
};

class AudioEventListener
{
public:
	virtual void AudioEvent(float time, const std::string& text) {};
};
