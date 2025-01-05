#include "Town.h"
#include "Framebuffer.h"
#include "Background.h"
#include "Iris.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Model.h"

static std::array<ModelP, 64> tileModels;

float Map::GetHeight(const glm::vec3& pos)
{
	/*
	TODO: check if there's a building, special acre, or placed object here.
	If so, use proper ray casting to find out the answer.
	*/
	int tx = (int)pos.x;
	int ty = (int)pos.y;
	auto tile = Terrain[tx + (ty * Width)];
	return (float)tile.Elevation * ElevationHeight;
}

float Map::GetHeight(int x, int y)
{
	return GetHeight(glm::vec3(x, y, 100));
}

extern float timeScale;
extern bool wireframe, postFx;
extern Shader* modelShader;
extern Shader* skyShader;
extern Background* rainLayer;
extern Framebuffer* frameBuffer;

extern unsigned int commonBuffer;

float lastGrassColor = 100.0f;

TextureArray* groundTextureAlbs{ nullptr };
TextureArray* groundTextureNrms{ nullptr };
TextureArray* groundTextureMixs{ nullptr };
TextureArray* grassColors{ nullptr };
TextureArray* snowMix{ nullptr };

static void LoadModels()
{
	//TODO: use a JSON file
	tileModels[0] = std::make_shared<::Model>("field/ground/unit.fbx");
	tileModels[1] = std::make_shared<::Model>("field/ground/cliff-12346789.fbx");
	tileModels[2] = std::make_shared<::Model>("field/ground/cliff-147.fbx");
	tileModels[3] = std::make_shared<::Model>("field/ground/cliff-14789.fbx");
	tileModels[4] = std::make_shared<::Model>("field/ground/cliff-1.fbx");
}

static void UpdateGrass()
{
	if (glm::abs(lastGrassColor - commonUniforms.GrassColor) < glm::epsilon<float>())
		return;

	lastGrassColor = commonUniforms.GrassColor;
	auto newMix = groundTextureMixs;
	if (commonUniforms.GrassColor <= 0.052f || commonUniforms.GrassColor >= 0.865f)
		newMix = snowMix;

	for (auto& model : tileModels)
	{
		if (!model)
			continue;

		for (auto& mesh : model->Meshes)
		{
			if (mesh.Name.find("_mGrass") != std::string::npos)
			{
				mesh.Textures[0] = groundTextureAlbs;
				mesh.Textures[1] = groundTextureNrms;
				mesh.Textures[2] = newMix;
			}
		}
	}

}

void Map::WorkOutModels()
{
	if (!tileModels[0])
		LoadModels();

	//TODO
}

#ifdef DEBUG
#include "support/stb_image_write.h"
void Map::SaveToPNG()
{
	auto width = Width * 2;
	auto height = Height * 2;
	auto pixels = new unsigned long[width * height * 4];
	const glm::vec4 typeColors[] =
	{
		{ 0.133, 0.545, 0.133, 1.0 },
		{ 0.81, 0.59, 0.28, 1.0 }
	};

	auto set = [pixels, width](glm::vec4 color, int x, int y, int w, int h)
	{
		auto r = (unsigned char)(color.r * 255);
		auto g = (unsigned char)(color.g * 255);
		auto b = (unsigned char)(color.b * 255);
		auto c = (r) | (g << 8) | (b << 16) | (255 << 24);

		for (int _y = y; _y < y + h; _y++)
			for (int _x = x; _x < x + w; _x++)
				pixels[(_y * width) + _x] = c;
	};

	for (int i = 0; i < Width * Height; i++)
	{
		auto t = Terrain[i];
		auto color = typeColors[t.Type];
		color = glm::mix(color, glm::vec4(1), t.Elevation * 0.25f);
		set(color, (i % Width) * 2, (i / Width) * 2, 2, 2);
	}

	/*
	Objects on the map should have a size in tiles and a color.
	If the color's alpha is zero, the object is to be skipped.
	*/
	set(glm::vec4(0.75), 1, 1, 5, 3);

	stbi_write_png("map.png", width, height, 4, pixels, width * 4);
}
#endif

Town::Town()
{
	Music = "clock";
	CanOverrideMusic = true;

	weatherSeed = std::rand();

	Width = AcreSize * 1;
	Height = AcreSize * 1;

	//TEST
	Terrain = std::make_unique<LiveTerrainTile[]>(Width * Height);
	for (int i = 0; i < Width; i++)
	{
		//Top
		Terrain[i].Elevation = 1;
		Terrain[i].Model = 2;
		Terrain[i].Rotation = 1;
	}
	for (int i = 0; i < Height; i++)
	{
		//Left
		Terrain[(i * Width)].Elevation = 1;
		Terrain[(i * Width)].Model = 2;
		Terrain[(i * Width)].Rotation = 2;
		//Right
		Terrain[(i * Width) + (Width - 1)].Elevation = 1;
		Terrain[(i * Width) + (Width - 1)].Model = 2;
		Terrain[(i * Width) + (Width - 1)].Rotation = 0;
	}
	Terrain[0].Model = 4; //left corner
	Terrain[0].Rotation = 1;
	Terrain[Width - 1].Model = 4; //right corner
	Terrain[Width - 1].Rotation = 0;

	Terrain[(4 * Width) + 2].Type = 1; //single sand tile
	Terrain[(0 * Width) + 2].Type = 2; //single stone tile on cliff
	
	Terrain[(8 * Width) + 5].Model = 1; //single column
	Terrain[(8 * Width) + 5].Elevation = 2;
	Terrain[(7 * Width) + 4].Model = 3; //surroundings
	Terrain[(7 * Width) + 4].Elevation = 1;
	Terrain[(7 * Width) + 4].Rotation = 0;
	Terrain[(7 * Width) + 5].Model = 2;
	Terrain[(7 * Width) + 5].Elevation = 1;
	Terrain[(7 * Width) + 5].Rotation = 3;
	Terrain[(7 * Width) + 6].Model = 3;
	Terrain[(7 * Width) + 6].Elevation = 1;
	Terrain[(7 * Width) + 6].Rotation = 3;
	Terrain[(8 * Width) + 4].Model = 2;
	Terrain[(8 * Width) + 4].Elevation = 1;
	Terrain[(8 * Width) + 4].Rotation = 0;
	Terrain[(8 * Width) + 6].Model = 2;
	Terrain[(8 * Width) + 6].Elevation = 1;
	Terrain[(8 * Width) + 6].Rotation = 2;
	Terrain[(9 * Width) + 4].Model = 3;
	Terrain[(9 * Width) + 4].Elevation = 1;
	Terrain[(9 * Width) + 4].Rotation = 1;
	Terrain[(9 * Width) + 5].Model = 2;
	Terrain[(9 * Width) + 5].Elevation = 1;
	Terrain[(9 * Width) + 5].Rotation = 1;
	Terrain[(9 * Width) + 6].Model = 3;
	Terrain[(9 * Width) + 6].Elevation = 1;
	Terrain[(9 * Width) + 6].Rotation = 2;

	//end test

	UseDrum = true;

#ifdef DEBUG
	SaveToPNG();
#endif
}

void Town::GenerateNew(void* generator, int width, int height)
{
	generator;

	Width = AcreSize * width;
	Height = AcreSize * height;

	Terrain = std::make_unique<LiveTerrainTile[]>(Width * Height);

#ifdef DEBUG
	//SaveToPNG();
#endif
}

void Town::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("map/flags.json");
		flags.clear();
		for (const auto& f : json->AsObject())
		{
			flags[f.first] = f.second->AsInteger();
		}

	}
	catch (std::runtime_error&)
	{ //-V565
		//Nothing to load.
		//TODO: DWI
	}

	WorkOutModels();
}

void Town::Save()
{
	JSONObject json;
	for (const auto& i : flags)
	{
		json[i.first] = new JSONValue(i.second);
	}
	auto val = JSONValue(json);
	VFS::WriteSaveJSON("map/flags.json", &val);
}

void Town::StartNewDay()
{
	//Select weather
	{
		tm gm;
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		const auto month = gm.tm_mon + 1;
		const auto day = gm.tm_mday;
		debprint(0, "Today is {} {}. Let's see.", day, month);

		std::srand(weatherSeed + (month << 8) + (day << 16));
		
		auto doc = VFS::ReadJSON("weather.json");
		auto json = doc->AsObject();
		auto calendar = json[Hemisphere == Hemisphere::North ? "north" : "south"]->AsArray();

		auto here = calendar[0]->AsObject();
		const int calNow = (month * 31) + day;
		//NOTE: We assume all entries are sorted by date.
		for (int i = 1; 0 < calendar.size(); i++)
		{
			auto c = calendar[i]->AsObject(); //-V836 TODO: figure this out
			const auto until = GetJSONVec2(c["until"]);
			const int calHere = ((int)until[1] * 31) + (int)until[0];
			if (calHere > calNow)
			{
				here = calendar[i]->AsObject();
				break;
			}
		}

		//Now to pick a weather pattern for this day...
		std::srand(weatherSeed + calNow);

		std::vector<int> rates;
		int rateTotal = 0;
		for (auto r : here["rates"]->AsArray())
		{
			rateTotal += r->AsInteger();
			rates.push_back(r->AsInteger());
		}
		auto roll = std::rand() % rateTotal;
		auto pick = 0;
		for (int i = 0; i < rates.size(); i++)
		{
			if (roll < rates[i])
			{
				pick = i;
				break;
			}
			roll -= rates[i];
		}

		auto patterns = json["patterns"]->AsArray();
		auto pattern = patterns[pick]->AsObject();
		debprint(0, "Weather: picked {}, \"{}\".", pick, pattern["id"]->AsString());
		auto rain = pattern["rain"]->AsArray();
		auto wind = pattern["wind"]->AsArray();
		for (int i = 0; i < 24; i++)
		{
			weatherRain[i] = rain[i]->AsInteger();
			weatherWind[i] = wind[i]->AsInteger();
		}

		delete doc;
	}
}

void Town::UpdateWeather()
{
	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	auto hour = gm.tm_hour;

	Clouds = static_cast<Town::Weather>(weatherRain[hour]);
	
	if (weatherWind[hour] == 0)
		Wind = 0;
	else
	{
		auto baseWind = std::abs(weatherWind[hour]) - 1;
		auto landIsEast = ((weatherSeed >> 16) % 2) == 1;
		auto windIsLand = weatherWind[hour] < 0;
		auto windStrength = ((1 << baseWind) - 1) + (std::rand() % 3);
		if (windIsLand && landIsEast)
			windStrength = -windStrength;
		Wind = windStrength;
	}
}

void Town::SetFlag(const std::string& id, int value)
{
	flags[id] = value;
}

void Town::SetFlag(const std::string& id, bool value)
{
	SetFlag(id, (int)value);
}

int Town::GetFlag(const std::string& id, int def)
{
	auto v = flags.find(id);
	if (v == flags.end())
		return def;
	return v->second;
}

bool Town::GetFlag(const std::string& id, bool def)
{
	return GetFlag(id, (int)def) > 0;
}

void Town::drawWorker(float dt)
{
	Sprite::DrawSprite(skyShader, *whiteRect, glm::vec2(0), glm::vec2(width, height));
	Sprite::FlushBatch();

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	//modelShader->Use();

	UpdateGrass();

	thePlayer.Draw(dt * timeScale);
	MeshBucket::Flush();
	for (const auto& v : town.Villagers)
		v->Draw(dt * timeScale);
	MeshBucket::Flush();

	//just doing a single _fake_ acre, no whammies...
	for (int y = 0; y < AcreSize; y++)
	{
		for (int x = 0; x < AcreSize; x++)
		{
			auto tile = Terrain[(y * Width) + x];
			auto model = tileModels[tile.Model];
			auto pos = glm::vec3(x * 10, tile.Elevation * ElevationHeight, y * 10);
			auto rot = tile.Rotation * 90.0f;
			if (tile.Model == 0 || tile.Model > 44)
				model->SetLayerByMat("_mGrass", tile.Type); //ground, river, or waterfall.
			else if (tile.Model < 44)
				model->SetLayer("GrassT__mGrass", tile.Type); //cliffs should only have the top grass changed.
			model->Draw(pos, rot);
		}
	}
	MeshBucket::Flush();

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if ((int)Clouds >= (int)Town::Weather::RainClouds)
		rainLayer->Draw(dt * timeScale);

	Sprite::FlushBatch();
}

void Town::Draw(float dt)
{
	if (groundTextureAlbs == nullptr)
	{
		std::vector<std::string> groundAlbs, groundNrms, groundMixs;
		for (auto& d : VFS::Enumerate("field/ground/design*_alb.png"))
			groundAlbs.push_back(d.path);
		for (auto& d : VFS::Enumerate("field/ground/design*_nrm.png"))
			groundNrms.push_back(d.path);
		for (auto& d : VFS::Enumerate("field/ground/design*_mix.png"))
			groundMixs.push_back(d.path);
		//also add user designs somehow.
		groundTextureAlbs = new TextureArray(groundAlbs);
		groundTextureNrms = new TextureArray(groundNrms);
		groundTextureMixs = new TextureArray(groundMixs);
		grassColors = new TextureArray("field/ground/grasscolors.png", GL_CLAMP_TO_EDGE, GL_NEAREST);
		groundMixs[0] = "field/ground/snow_mix.png";
		snowMix = new TextureArray(groundMixs);
	}

	if (!tileModels[0])
		LoadModels();
	
	if (postFx)
	{
		frameBuffer->Use();
		glClearColor(0.2f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawWorker(dt * timeScale);
		frameBuffer->Drop();
		frameBuffer->Draw();
	}
	else
	{
		drawWorker(dt * timeScale);
	}
	if (!iris->Done())
		iris->Draw(dt);
}

void Town::Tick(float dt)
{
	if (!iris->Done())
	{
		iris->Tick(dt);
		if (iris->Done() && itemHotbar && dateTimePanel)
		{
			itemHotbar->Show();
			dateTimePanel->Show();
		}
	}

	thePlayer.Tick(dt);
}

Town town;
