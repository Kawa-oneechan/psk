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

Audio::Audio(std::string filename)
{
	this->theSound = nullptr;
	this->theChannel = nullptr;
	size_t size = 0;
	this->filename = filename;
	if (!audioEnabled)
	{
		this->status = AudioStatus::Invalid;
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
	auto r = system->createStream(data, FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D | FMOD_OPENMEMORY, &soundEx, &this->theSound);
	if (r != FMOD_OK)
	{
		fmt::format("Could not create stream for audio file {}.", filename);
		return;
	}
	auto ext = filename.substr(filename.length() - 4, 4);
	if (ext == ".ogg")
	{
		FMOD_TAG tag;
		r = this->theSound->getTag("LOOP_START", 0, &tag);
		if (r == FMOD_OK)
		{
			unsigned int start = atoi((char*)tag.data);
			unsigned int end = 0;
			this->theSound->getLength(&end, FMOD_TIMEUNIT_PCM);
			r = this->theSound->setLoopPoints(start, FMOD_TIMEUNIT_PCM, end, FMOD_TIMEUNIT_PCM);
			if (r != FMOD_OK)
				fmt::print("Wanted to set loop point for file {}, could not.", filename);
		}
	}
	this->status = AudioStatus::Stopped;
}

Audio::~Audio()
{
	this->Stop();
	if (audioEnabled)
		this->theSound->release();
	this->theChannel = NULL;
	this->theSound = NULL;
}

void Audio::Play()
{
	if (this->status == AudioStatus::Stopped)
	{
		if (audioEnabled)
		{
			auto r = system->playSound(FMOD_CHANNEL_FREE, this->theSound, false, &this->theChannel);
			if (r != FMOD_OK)
				throw "Could not play stream.";
			this->theChannel->setVolume(musicVolume);
		}
	}
	else if (this->status == AudioStatus::Paused)
	{
		this->theChannel->setPaused(false);
	}
	this->status = AudioStatus::Playing;
}

void Audio::Pause()
{
	if (audioEnabled)
		this->theChannel->setPaused(true);
	this->status = AudioStatus::Paused;
}

void Audio::Stop()
{
	if (this->status != AudioStatus::Stopped)
	{
		if (audioEnabled)
			this->theChannel->stop();
	}
	this->status = AudioStatus::Stopped;
}
