#include "Audio.h"

FMOD::System* Audio::system;
std::vector<Audio*> Audio::playing;

std::map<std::string, std::shared_ptr<Audio>> uiSounds;

bool Audio::Enabled;
float Audio::MusicVolume, Audio::AmbientVolume, Audio::SoundVolume, Audio::SpeechVolume;

void Audio::Initialize()
{
	Enabled = true;
	auto r = FMOD::System_Create(&system);
	if (r != FMOD_OK)
	{
		conprint(1, "Could not create FMOD system object. Sound disabled.");
		Enabled = false;
		return;
	}
	r = system->init(4, FMOD_INIT_NORMAL, NULL);
	if (r != FMOD_OK)
	{
		conprint(1, "Could not initialize FMOD system object. Sound disabled.");
		Enabled = false;
		return;
	}
}

void Audio::Update()
{
	system->update();

	static auto epsilon = glm::epsilon<float>();
	static auto oldMusicVolume = MusicVolume;
	auto changed = false;

	if (std::fabs(oldMusicVolume - MusicVolume) > epsilon)
	{
		oldMusicVolume = MusicVolume;
		changed = true;
	}

	if (changed)
	{
		for (auto x : playing)
		{
			x->UpdateVolume();
		}
	}
}

static FMOD_RESULT F_CALLBACK callback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
	return FMOD_OK;
}

Audio::Audio(std::string filename) : filename(filename)
{
	size_t size = 0;
	if (!Enabled)
	{
		status = Status::Invalid;
		return;
	}
	data = ReadVFS(filename, &size);
	if (!data)
	{
		conprint(1, "Could not open audio file {}.", filename);
		return;
	}
	auto soundEx = FMOD_CREATESOUNDEXINFO();
	memset(&soundEx, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	soundEx.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	soundEx.length = (unsigned int)size;
	auto mode = FMOD_HARDWARE | FMOD_2D | FMOD_OPENMEMORY;
	//this needs a better detection method
	if (filename.substr(0, 5) == "music")
		type = Type::Music;
	else if (filename.substr(0, 7) == "ambient")
		type = Type::Ambient;
	else if (filename.substr(6, 9) == "animalese")
		type = Type::Speech;
	else
		type = Type::Sound; //TODO: extend
	auto r = system->createStream(data.get(), mode, &soundEx, &theSound);
	if (r != FMOD_OK)
	{
		fmt::format("Could not create stream for audio file {}.", filename);
		return;
	}
	theChannel->setCallback(callback);
	auto ext = filename.substr(filename.length() - 4, 4);
	if (ext == ".ogg")
	{
		FMOD_TAG tag;
		r = theSound->getTag("LOOP_START", 0, &tag);
		if (r == FMOD_OK)
		{
			r = theSound->setMode(mode | FMOD_LOOP_NORMAL);
			if (r != FMOD_OK)
				conprint(1, "Wanted to enable looping for file {}, could not.", filename);
			unsigned int start = atoi((char*)tag.data);
			unsigned int end = 0;
			theSound->getLength(&end, FMOD_TIMEUNIT_PCM);
			r = theSound->setLoopPoints(start, FMOD_TIMEUNIT_PCM, end, FMOD_TIMEUNIT_PCM);
			if (r != FMOD_OK)
				conprint(1, "Wanted to set loop point for file {}, could not.", filename);
		}
	}
	status = Status::Stopped;
}

Audio::~Audio()
{
	Stop();
	if (Enabled)
		theSound->release();
	theChannel = NULL;
	theSound = NULL;
}

void Audio::Play(bool force)
{
	if (force && status != Status::Stopped)
		Stop();

	if (status == Status::Stopped)
	{
		if (Enabled)
		{
			auto r = system->playSound(FMOD_CHANNEL_FREE, theSound, false, &theChannel);
			if (r != FMOD_OK)
				throw "Could not play stream.";
			UpdateVolume();

		}
		playing.push_back(this);
	}
	else if (status == Status::Paused)
	{
		theChannel->setPaused(false);
	}
	theChannel->getFrequency(&frequency);
	status = Status::Playing;
}

void Audio::Pause()
{
	if (Enabled)
		theChannel->setPaused(true);
	status = Status::Paused;
}

void Audio::Stop()
{
	if (status != Status::Stopped)
	{
		if (Enabled)
			theChannel->stop();
	}
	status = Status::Stopped;
	playing.erase(std::remove(playing.begin(), playing.end(), this), playing.end());
}

void Audio::UpdateVolume()
{
	auto v = 0.0f;
	switch (type)
	{
	case Type::Music: v = MusicVolume; break;
	case Type::Ambient: v = AmbientVolume; break;
	case Type::Sound: v = SoundVolume; break;
	case Type::Speech: v = SpeechVolume; break;
	}
	theChannel->setVolume(v * Volume);
}

void Audio::SetPitch(float ratio)
{
	theChannel->setFrequency(frequency * ratio);
}

void Audio::SetPosition(glm::vec3 pos)
{
	//Only generic sounds can be positioned.
	if (type != Type::Sound)
		return;
	FMOD_VECTOR v = {pos.x, pos.y, pos.z};
	theChannel->set3DAttributes(&v, nullptr);
}

void Audio::SetPan(float pos)
{
	auto r = theChannel->setPan(pos);
	if (r != FMOD_OK)
		conprint(1, "Couldn't set pan position for {}.", filename);
}
