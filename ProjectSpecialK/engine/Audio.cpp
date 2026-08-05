#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Audio.h"
#include "Console.h"
#include "VFS.h"
#include "../Game.h"

static SoLoud::Soloud sys;
static std::vector<AudioSoundBase*> playing;

bool Audio::Enabled;
float Audio::MusicVolume, Audio::SoundVolume;
#ifdef BECKETT_MOREVOLUME
float Audio::AmbientVolume, Audio::SpeechVolume;
#endif

static auto epsilon = glm::epsilon<float>();

void Audio::Initialize()
{
	Enabled = true;
	if (sys.init() != 0)
	{
		conprint(1, "Could not initialize SoLoud. Sound disabled.");
		Enabled = false;
		return;
	}

#ifdef BECKETT_3DAUDIO
	sys.set3dListenerUp(0.0f, 0.1f, 0.0f);
#endif
}

void Audio::Update()
{
	static auto oldMusicVolume = MusicVolume;
	auto changed = false;

	if (std::fabs(oldMusicVolume - MusicVolume) > epsilon)
	{
		oldMusicVolume = MusicVolume;
		changed = true;
	}

	for (auto x : playing)
	{
		x->update();
		if (changed)
		{
			x->UpdateVolume();
		}
	}

#ifdef BECKETT_3DAUDIO
	sys.update3dAudio();
#endif
}

static std::map<std::string, std::string> getTags(char* data)
{
	auto ret = std::map<std::string, std::string>();

	//Try to find the loop start tag by hand.
	{
		char* tagStart = nullptr;
		for (size_t i = 0; i < 1024; i++)
		{
			if (data[i + 0] == 0x03 && //comment block
				data[i + 1] == 'v' &&
				data[i + 2] == 'o' &&
				data[i + 3] == 'r' &&
				data[i + 4] == 'b' &&
				data[i + 5] == 'i' &&
				data[i + 6] == 's')
				tagStart = data + i + 7;
		}
		if (tagStart)
		{
			//Couldn't *not* find the comment block, there has to be a vendor string. But what the hell.
			auto cursor = data + 0x28;

			auto readInt = [&]()
			{
				auto a = (unsigned char)*cursor; cursor++;
				auto b = (unsigned char)*cursor; cursor++;
				auto c = (unsigned char)*cursor; cursor++;
				auto d = (unsigned char)*cursor; cursor++;
				return (a << 0) | (b << 8) | (c << 16) | (d << 24);
			};
			auto readString = [&]()
			{
				auto len = readInt();
				std::string ret;
				ret.reserve(len);
				ret.append(cursor, len);
				cursor += len;
				return ret;
			};

			auto sampleRate = readInt();
			ret["SAMPLE_RATE"] = fmt::format("{}", sampleRate);

			cursor = tagStart;

			auto vendor = readString();
			auto numTags = readInt();
			for (int i = 0; i < numTags; i++)
			{
				auto tag = readString();
				auto parts = Split(tag, '=');
				auto key = parts[0];
				auto value = parts[1];
				ret[key] = value;
			}
		}
	}
	return ret;
}

Sound::Sound(const std::string& filename, SoundType type)
{
	AudioSoundBase::filename = filename;
	AudioSoundBase::type = type;

	size_t size = 0;
	if (!Audio::Enabled)
	{
		status = Status::Invalid;
		return;
	}
	data = VFS::ReadData(filename, &size);
	if (!data)
	{
		conprint(1, "Could not open audio file {}.", filename);
		return;
	}
	
	if (sound.loadMem(reinterpret_cast<unsigned char*>(data.get()), (unsigned int)size, true) != 0)
	{
		fmt::format("Could not create sound for audio file {}.", filename);
		return;
	}

	{
		auto tags = getTags(data.get());
		if (!tags["LOOP_START"].empty())
		{
			auto loopPoint = std::stof(tags["LOOP_START"]);
			auto sampleRate = std::stof(tags["SAMPLE_RATE"]);
			sound.setLoopPoint(loopPoint / sampleRate);
			sound.setLooping(true);
		}
	}

	{
		auto maybeTagFile = VFS::ChangeExtension(filename, "txt");
		auto maybeTags = VFS::ReadString(maybeTagFile);
		if (!maybeTags.empty())
		{
			//parse Audacity tag file
			ReplaceAll(maybeTags, "\r", "");
			auto lines = Split(maybeTags, '\n');
			for (auto& line : lines)
			{
				auto parts = Split(line, '\t');
				auto time = std::stof(parts[0]);
				auto text = parts.size() > 2 ? parts[2] : "";
				tags.push_back(std::make_tuple(time, text));
			}
			nextTag = std::get<0>(tags[0]);
		}
	}
	
	status = Status::Stopped;
}

Stream::Stream(const std::string& filename, SoundType type)
{
	AudioSoundBase::filename = filename;
	AudioSoundBase::type = type;

	size_t size = 0;
	if (!Audio::Enabled)
	{
		status = Status::Invalid;
		return;
	}
	data = VFS::ReadData(filename, &size);
	if (!data)
	{
		conprint(1, "Could not open audio file {}.", filename);
		return;
	}

	if (stream.loadMem(reinterpret_cast<unsigned char*>(data.get()), (unsigned int)size, true) != 0)
	{
		fmt::format("Could not create stream for audio file {}.", filename);
		return;
	}

	{
		auto tags = getTags(data.get());
		if (!tags["LOOP_START"].empty())
		{
			auto loopPoint = std::stof(tags["LOOP_START"]);
			auto sampleRate = std::stof(tags["SAMPLE_RATE"]);
			stream.setLoopPoint(loopPoint / sampleRate);
			stream.setLooping(true);
		}
		Name = tags["title"];
		Artist = tags["artist"];
	}

	auto maybeTagFile = VFS::ChangeExtension(filename, "txt");
	auto maybeTags = VFS::ReadString(maybeTagFile);
	if (!maybeTags.empty())
	{
		//parse Audacity tag file
		ReplaceAll(maybeTags, "\r", "");
		auto lines = Split(maybeTags, '\n');
		for (auto& line : lines)
		{
			auto parts = Split(line, '\t');
			auto time = std::stof(parts[0]);
			auto text = parts.size() > 2 ? parts[2] : "";
			tags.push_back(std::make_tuple(time, text));
		}
		nextTag = std::get<0>(tags[0]);
	}

	status = Status::Stopped;
}

Sound::~Sound()
{
	Stop();
}

Stream::~Stream()
{
	Stop();
}

void Audio::SetListenerPosition(const glm::vec3& pos)
{
#ifdef BECKETT_3DAUDIO
	sys.set3dListenerPosition(pos.x, pos.y, pos.z);
#endif
	return;
}

#ifndef BECKETT_3DAUDIO
void Sound::Play(bool force)
#else
void Sound::Play(bool force, bool in3D)
#endif
{
	if (force && status != Status::Stopped)
		Stop();

	if (status == Status::Stopped)
	{
		if (Audio::Enabled)
		{
#ifdef BECKETT_3DAUDIO
			is3D = in3D;
			if (in3D)
			{
				handle = sys.play3d(sound, 0.0f, 0.0f, 0.0f);
			}
			else
#endif
			{
				UpdateVolume();
				handle = sys.play(sound, volume, panPot);
			}
		}
		playing.push_back(this);
	}
	else if (status == Status::Paused)
	{
		sys.setPause(handle, false);
	}
	status = Status::Playing;
}

#ifndef BECKETT_3DAUDIO
void Stream::Play(bool force)
#else
void Stream::Play(bool force, bool in3D)
#endif
{
	if (force && status != Status::Stopped)
		Stop();

	if (status == Status::Stopped)
	{
		if (Audio::Enabled)
		{
#ifdef BECKETT_3DAUDIO
			is3D = in3D;
			if (in3D)
			{
				handle = sys.play3d(stream, 0.0f, 0.0f, 0.0f);
			}
			else
#endif
			{
				UpdateVolume();
				handle = sys.play(stream, volume, panPot);
			}
		}
		playing.push_back(this);
	}
	else if (status == Status::Paused)
	{
		sys.setPause(handle, false);
	}
	status = Status::Playing;
}

void AudioSoundBase::Pause()
{
	if (Audio::Enabled)
		sys.setPause(handle, true);
	status = Status::Paused;
}

void AudioSoundBase::Stop()
{
	if (status != Status::Stopped)
	{
		if (Audio::Enabled)
			sys.stop(handle);
	}
	status = Status::Stopped;
	playing.erase(std::remove(playing.begin(), playing.end(), this), playing.end());
}

void Sound::update()
{}

void Stream::update()
{
	if (tags.empty())
		return;
	if (listeners.empty())
		return;
	float fpos = (float)sys.getStreamPosition(handle);
	float flen = (float)stream.getLength();
	fpos = glm::mod(fpos, flen);

	if (currentTag >= tags.size())
	{
		if (fpos <= nextTag)
			currentTag = 0;
		else
			return;
	}
	while (fpos > nextTag)
	{
		for (auto listener : listeners)
		{
			listener->AudioEvent(lastTag, std::get<1>(tags[currentTag]));
		}
		currentTag++;
		if (currentTag >= tags.size())
		{
			currentTag = 99999;
			lastTag = -1;
			nextTag = std::get<0>(tags[0]);
			return;
		}
		lastTag = nextTag;
		nextTag = std::get<0>(tags[currentTag]);
	}
}

void AudioSoundBase::UpdateVolume()
{
	auto v = 0.0f;
	switch (type)
	{
	case SoundType::Music: v = Audio::MusicVolume; break;
	case SoundType::Sound: v = Audio::SoundVolume; break;
#ifdef BECKETT_MOREVOLUME
	case SoundType::Ambient: v = Audio::AmbientVolume; break;
	case SoundType::Speech: v = Audio::SpeechVolume; break;
#endif
	}
	volume = glm::clamp(v * Volume, 0.0f, 1.0f);
	sys.setVolume(handle, volume);
}

void AudioSoundBase::SetPitch(float ratio)
{
	assert(ratio > 0.0f);
	sys.setRelativePlaySpeed(handle, ratio);
}

void AudioSoundBase::SetPosition(const glm::vec3& pos)
{
#ifdef BECKETT_3DAUDIO
	if (is3D)
		sys.set3dSourceParameters(handle, pos.x, pos.y, pos.z, 0.0f, 0.0f, 0.0f);
#endif
}

void AudioSoundBase::SetPan(float pos)
{
	panPot = glm::clamp(pos, -1.0f, 1.0f);
	sys.setPan(handle, panPot);
}

void Sound::SetLoop(bool loop)
{
	sound.setLooping(loop);
}

void Stream::SetLoop(bool loop)
{
	stream.setLooping(loop);
}

void AudioSoundBase::RegisterListener(const AudioEventListener* listener)
{
	if (std::find(listeners.cbegin(), listeners.cend(), listener) != listeners.cend())
		return;
	listeners.push_back(const_cast<AudioEventListener*>(listener));
}

void AudioSoundBase::UnregisterListener(const AudioEventListener* listener)
{
	auto it = std::find_if(listeners.begin(), listeners.end(), [listener](auto e)
	{
		return e == listener;
	});
	if (it != listeners.end())
		listeners.erase(it);
}
