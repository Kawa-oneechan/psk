#include "MusicManager.h"
#include "Town.h"

void PlayMusic(const std::string& id)
{
	musicManager->Play(id, true);
}

MusicManager::MusicManager()
{
}

bool MusicManager::Tick(float dt)
{
	if (state == MusicState::FadeOut || state == MusicState::FadeToQueue)
	{
		if (!bgm)
		{
			state = MusicState::Idle;
			return true;
		}
		if (bgm->Volume > 0.0f)
		{
			bgm->Volume -= dt * 1.0f;
			bgm->UpdateVolume();
		}
		if (bgm->Volume <= 0.0f)
		{
			if (state == MusicState::FadeOut)
			{
				bgm->Stop();
				state = MusicState::Idle;
			}
			else //if (state == MusicState::FadeToQueue)
			{
				Play(queued, true);
			}
		}
	}
	return true;
}

void MusicManager::Draw(float)
{
	//Music doesn't visual.
}

void MusicManager::Play(const std::string& id, bool immediate)
{
	if (id == currentID)
		return;

	if (library.size() == 0)
		library = VFS::ReadJSON("music/music.json")->AsObject();
	//don't bother deleting it here and now, we're holding onto this.

	if (!immediate && !currentID.empty())
	{
		queued = id;
		state = MusicState::FadeToQueue;
		return;
	}

	if (id.empty())
	{
		bgm.reset();
		currentFile.clear();
		currentID.clear();
		return;
	}

	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);

	auto entry = library.find(id);
	if (entry == library.end())
	{
		if (id != "fallback")
			PlayMusic("fallback");
		return;
	}

	auto second = entry->second;

	if (second->IsArray() && second->AsArray()[0]->IsObject())
	{
		auto now2 = (gm.tm_hour * 24) + gm.tm_min;
		auto prev = 0;
		for (auto& ranges : second->AsArray())
		{
			auto r = ranges->AsObject();
			auto t = GetJSONVec2(r["to"]);
			auto to = (int)((t[0] * 24) + t[1]);
			if (now2 < to && now2 > prev)
			{
				second = r["file"];
			}
			prev = to;
		}
	}

	while (second->IsArray())
	{
		auto arr = second->AsArray();
		second = arr[rnd::getInt((int)arr.size())];
	}

	if (!second->IsString())
	{
		conprint(1, "PlayMusic: could not figure out \"{}\", did not end up with a string.", id);
		return;
	}

	auto file = second->AsString();

	{
		auto weather = "sunny";
		if (town->Clouds >= Town::Weather::RainClouds)
			weather = "rainy";
		auto tpos = file.find("{time}");
		if (tpos != std::string::npos)
			file = file.replace(tpos, 6, fmt::format("{:02}", gm.tm_hour));

		{
			auto preWeather = file;
			tpos = file.find("{weather}");
			if (tpos != std::string::npos)
				file = file.replace(tpos, 9, weather);

			if (VFS::Enumerate(file).size() == 0)
			{
				//File does not exist. Try sunny weather first.
				file = preWeather.replace(tpos, 9, "sunny");
				if (VFS::Enumerate(file).size() == 0)
				{
					//That too? Okay, try *no* weather!
					file = preWeather.replace(tpos, 9, "");
				}
			}
		}
	}

	if (currentFile != file)
	{
		bgm = std::make_shared<Audio>(file);
		bgm->Play();
		state = MusicState::Playing;
	}

	currentFile = file;
	currentID = id;
}

void MusicManager::FadeOut()
{
	Play("");
}
