#pragma once

#include "engine/Audio.h"
#include "engine/Tickable.h"

class MusicManager : public Tickable
{
private:
	std::string queued;

	enum class MusicState
	{
		Idle, Playing, FadeOut, FadeToQueue
	} state{ MusicState::Idle };

	jsonObject library = jsonObject();
	std::string currentID;
	std::string currentFile;

	std::shared_ptr<Audio> bgm{ nullptr };

public:
	std::string Override;

	MusicManager();
	bool Tick(float dt) override;
	void Draw(float dt) override;
	void Play(const std::string& id, bool immediate = false, bool ignoreID = false);
	void FadeOut();

};

extern std::shared_ptr<MusicManager> musicManager;
