#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Audio.h"
#include "Console.h"
#include "VFS.h"
#include "../Game.h"

SoLoud::Soloud Audio::system;
std::vector<Audio*> Audio::playing;

bool Audio::Enabled;
float Audio::MusicVolume, Audio::SoundVolume;
#ifdef BECKETT_MOREVOLUME
float Audio::AmbientVolume, Audio::SpeechVolume;
#endif

static auto epsilon = glm::epsilon<float>();

void Audio::Initialize()
{
	Enabled = true;
	if (system.init() != 0)
	{
		conprint(1, "Could not initialize SoLoud. Sound disabled.");
		Enabled = false;
		return;
	}

#ifdef BECKETT_3DAUDIO
	system.set3dListenerUp(0.0f, 0.1f, 0.0f);
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
	system.update3dAudio();
#endif
}

Audio::Audio(const std::string& filename) : filename(filename)
{
	size_t size = 0;
	if (!Enabled)
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
	if (filename.find("music/") != std::string::npos)
		type = Type::Music;
#ifdef BECKETT_MOREVOLUME
	else if (filename.find("ambient/") != std::string::npos)
		type = Type::Ambient;
	else if (filename.find("speech/") != std::string::npos)
		type = Type::Speech;
#endif
	else
		type = Type::Sound;

#ifndef BECKETT_MOREVOLUME
	isStream = (type == Type::Music);
#else 
	isStream = (type == Type::Music) || (type == Type::Ambient);
#endif

	if (isStream)
	{
		if (stream.loadMem(reinterpret_cast<unsigned char*>(data.get()), (unsigned int)size, true) != 0)
		{
			fmt::format("Could not create stream for audio file {}.", filename);
			return;
		}
	}
	else
	{
		if (sound.loadMem(reinterpret_cast<unsigned char*>(data.get()), (unsigned int)size, true) != 0)
		{
			fmt::format("Could not create sound for audio file {}.", filename);
			return;
		}
	}

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
				tagStart = data.get() + i + 7;
		}
		if (tagStart)
		{
			//Couldn't *not* find the comment block, there has to be a vendor string. But what the hell.
			auto cursor = data.get() + 0x28;

			auto readInt = [&]() {
				auto a = (unsigned char)*cursor; cursor++;
				auto b = (unsigned char)*cursor; cursor++;
				auto c = (unsigned char)*cursor; cursor++;
				auto d = (unsigned char)*cursor; cursor++;
				return (a << 0) | (b << 8) | (c << 16) | (d << 24);
			};
			auto readString = [&]() {
				auto len = readInt();
				std::string ret;
				ret.reserve(len);
				ret.append(cursor, len);
				cursor += len;
				return ret;
			};

			auto sampleRate = readInt();

			cursor = tagStart;

			auto vendor = readString();
			auto numTags = readInt();
			for (int i = 0; i < numTags; i++)
			{
				auto tag = readString();
				auto parts = Split(tag, '=');
				auto key = parts[0];
				auto value = parts[1];
				if (key == "LOOP_START")
				{
					auto time = std::stof(value);
					if (isStream)
					{
						stream.setLoopPoint(time / sampleRate);
						stream.setLooping(true);
					}
					else
					{
						sound.setLoopPoint(time / sampleRate);
						sound.setLooping(true);
					}
				}
			}
		}
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
			auto text = parts[2];
			tags.push_back(std::make_tuple(time, text));
		}
		nextTag = std::get<0>(tags[0]);
	}
	
	status = Status::Stopped;
}

Audio::~Audio()
{
	Stop();
}

void Audio::SetListenerPosition(const glm::vec3& pos)
{
#ifdef BECKETT_3DAUDIO
	system.set3dListenerPosition(pos.x, pos.y, pos.z);
#endif
	return;
}

#ifndef BECKETT_3DAUDIO
void Audio::Play(bool force)
#else
void Audio::Play(bool force, bool in3D)
#endif
{
	if (force && status != Status::Stopped)
		Stop();

	if (status == Status::Stopped)
	{
		if (Enabled)
		{
#ifdef BECKETT_3DAUDIO
			is3D = in3D;
			if (in3D)
			{
				if (isStream)
					handle = system.play3d(stream, 0.0f, 0.0f, 0.0f);
				else
					handle = system.play3d(sound, 0.0f, 0.0f, 0.0f);
			}
			else
#endif
			{
				UpdateVolume();

				if (isStream)
					handle = system.play(stream, volume, panPot);
				else
					handle = system.play(sound, volume, panPot);
			}
		}
		playing.push_back(this);
	}
	else if (status == Status::Paused)
	{
		system.setPause(handle, false);
	}
	status = Status::Playing;
}

void Audio::Pause()
{
	if (Enabled)
		system.setPause(handle, true);
	status = Status::Paused;
}

void Audio::Stop()
{
	if (status != Status::Stopped)
	{
		if (Enabled)
			system.stop(handle);
	}
	status = Status::Stopped;
	playing.erase(std::remove(playing.begin(), playing.end(), this), playing.end());
}

void Audio::update()
{
	if (!isStream)
		return;
	if (tags.empty())
		return;
	if (listeners.empty())
		return;
	if (currentTag >= tags.size())
		return;
	float fpos = (float)system.getStreamPosition(handle);
	
	while (fpos > nextTag)
	{
		for (auto listener : listeners)
		{
			listener->AudioEvent(lastTag, std::get<1>(tags[currentTag]));
		}
		currentTag++;
		if (currentTag >= tags.size())
		{
			currentTag = 0;
			lastTag = -1;
			nextTag = std::get<0>(tags[0]);
			return;
		}
		lastTag = nextTag;
		nextTag = std::get<0>(tags[currentTag]);
	}
}

void Audio::UpdateVolume()
{
	auto v = 0.0f;
	switch (type)
	{
	case Type::Music: v = MusicVolume; break;
	case Type::Sound: v = SoundVolume; break;
#ifdef BECKETT_MOREVOLUME
	case Type::Ambient: v = AmbientVolume; break;
	case Type::Speech: v = SpeechVolume; break;
#endif
	}
	volume = glm::clamp(v * Volume, 0.0f, 1.0f);
	system.setVolume(handle, volume);
}

void Audio::SetPitch(float ratio)
{
	assert(ratio > 0.0f);
	system.setRelativePlaySpeed(handle, ratio);
}

void Audio::SetPosition(const glm::vec3& pos)
{
#ifdef BECKETT_3DAUDIO
	if (is3D)
		system.set3dSourceParameters(handle, pos.x, pos.y, pos.z, 0.0f, 0.0f, 0.0f);
#endif
}

void Audio::SetPan(float pos)
{
	panPot = glm::clamp(pos, -1.0f, 1.0f);
	system.setPan(handle, panPot);
}

void Audio::SetLoop(bool loop)
{
	if (isStream)
		stream.setLooping(loop);
	else
		sound.setLooping(loop);
}

void Audio::RegisterListener(const AudioEventListener* listener)
{
	if (std::find(listeners.cbegin(), listeners.cend(), listener) != listeners.cend())
		return;
	listeners.push_back(const_cast<AudioEventListener*>(listener));
}

void Audio::UnregisterListener(const AudioEventListener* listener)
{
	auto it = std::find_if(listeners.begin(), listeners.end(), [listener](auto e)
	{
		return e == listener;
	});
	if (it != listeners.end())
		listeners.erase(it);
}
