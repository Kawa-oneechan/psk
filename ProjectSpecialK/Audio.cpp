#include "Audio.h"

FMOD::System* Audio::system;
std::vector<Audio*> Audio::playing;

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
	theSound = nullptr;
	theChannel = nullptr;
	Volume = 1.0f;
	size_t size = 0;
	type = 0;
	if (!Enabled)
	{
		status = AudioStatus::Invalid;
		return;
	}
	auto data = ReadVFS(filename, &size);
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
	if (filename.substr(0, 5) == "music")
	{
		mode |= FMOD_LOOP_NORMAL;
	}
	else
	{
		mode |= FMOD_LOOP_OFF;
		type = 1; //TODO: extend
	}
	auto r = system->createStream(data, mode, &soundEx, &theSound);
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
			unsigned int start = atoi((char*)tag.data);
			unsigned int end = 0;
			theSound->getLength(&end, FMOD_TIMEUNIT_PCM);
			r = theSound->setLoopPoints(start, FMOD_TIMEUNIT_PCM, end, FMOD_TIMEUNIT_PCM);
			if (r != FMOD_OK)
				conprint(1, "Wanted to set loop point for file {}, could not.", filename);
		}
	}
	status = AudioStatus::Stopped;
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
	if (force && status != AudioStatus::Stopped)
		Stop();

	if (status == AudioStatus::Stopped)
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
	else if (status == AudioStatus::Paused)
	{
		theChannel->setPaused(false);
	}
	status = AudioStatus::Playing;
}

void Audio::Pause()
{
	if (Enabled)
		theChannel->setPaused(true);
	status = AudioStatus::Paused;
}

void Audio::Stop()
{
	if (status != AudioStatus::Stopped)
	{
		if (Enabled)
			theChannel->stop();
	}
	status = AudioStatus::Stopped;
	playing.erase(std::remove(playing.begin(), playing.end(), this), playing.end());
}

void Audio::UpdateVolume()
{
	if (type == 0)
		theChannel->setVolume(MusicVolume * Volume);
	else if (type == 1)
		theChannel->setVolume(SoundVolume * Volume);
}
