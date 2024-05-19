#include "Audio.h"

FMOD::System* Audio::system;
bool audioEnabled = true;
float musicVolume = 1.0f, soundVolume = 1.0f;

void Audio::Initialize()
{
	if (audioEnabled)
	{
		auto r = FMOD::System_Create(&system);
		if (r != FMOD_OK)
		{
			fmt::print("Could not create FMOD system object. Sound disabled.");
			audioEnabled = false;
			return;
		}
		r = system->init(4, FMOD_INIT_NORMAL, NULL);
		if (r != FMOD_OK)
		{
			fmt::print("Could not initialize FMOD system object. Sound disabled.");
			audioEnabled = false;
			return;
		}
	}
}

Audio::Audio(std::string filename) : filename(filename)
{
	theSound = nullptr;
	theChannel = nullptr;
	size_t size = 0;
	if (!audioEnabled)
	{
		status = AudioStatus::Invalid;
		return;
	}
	auto data = ReadVFS(filename, &size);
	if (!data)
	{
		fmt::print("Could not open audio file {}.", filename);
		return;
	}
	auto soundEx = FMOD_CREATESOUNDEXINFO();
	memset(&soundEx, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	soundEx.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	soundEx.length = (unsigned int)size;
	auto mode = FMOD_HARDWARE | FMOD_2D | FMOD_OPENMEMORY;
	if (filename.substr(0, 5) == "music")
		mode |= FMOD_LOOP_NORMAL;
	else
		mode |= FMOD_LOOP_OFF;
	auto r = system->createStream(data, mode, &soundEx, &theSound);
	if (r != FMOD_OK)
	{
		fmt::format("Could not create stream for audio file {}.", filename);
		return;
	}
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
				fmt::print("Wanted to set loop point for file {}, could not.", filename);
		}
	}
	status = AudioStatus::Stopped;
}

Audio::~Audio()
{
	Stop();
	if (audioEnabled)
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
		if (audioEnabled)
		{
			auto r = system->playSound(FMOD_CHANNEL_FREE, theSound, false, &theChannel);
			if (r != FMOD_OK)
				throw "Could not play stream.";
			theChannel->setVolume(musicVolume);
		}
	}
	else if (status == AudioStatus::Paused)
	{
		theChannel->setPaused(false);
	}
	status = AudioStatus::Playing;
}

void Audio::Pause()
{
	if (audioEnabled)
		theChannel->setPaused(true);
	status = AudioStatus::Paused;
}

void Audio::Stop()
{
	if (status != AudioStatus::Stopped)
	{
		if (audioEnabled)
			theChannel->stop();
	}
	status = AudioStatus::Stopped;
}
