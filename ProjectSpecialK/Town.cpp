#include "Town.h"
#include "Framebuffer.h"
#include "Background.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Model.h"
#include <base64.h>

extern "C"
{
	extern int mz_compress(unsigned char *pDest, unsigned long *pDest_len, const unsigned char *pSource, unsigned long source_len);
	extern int mz_uncompress(unsigned char *pDest, unsigned long *pDest_len, const unsigned char *pSource, unsigned long source_len);
}

static std::array<ModelP, 80> tileModels;
static std::array<std::string, 80> tileModelKeys;

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
extern ColorMapBuffer* colorMapBuffer;

extern unsigned int commonBuffer;

float lastGrassColor = 100.0f;

TextureArray* groundTextureAlbs{ nullptr };
TextureArray* groundTextureNrms{ nullptr };
TextureArray* groundTextureMixs{ nullptr };
TextureArray* grassColors{ nullptr };
TextureArray* snowMix{ nullptr };
TextureArray* squareMix{ nullptr };

static void UpdateGrass()
{
	if (glm::abs(lastGrassColor - commonUniforms.GrassColor) < glm::epsilon<float>())
		return;

	lastGrassColor = commonUniforms.GrassColor;
	auto newMix = groundTextureMixs;
	if (town->SquareGrass)
		newMix = squareMix;
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
	std::map<std::string, std::tuple<std::string, int>> rules;

	auto json = VFS::ReadJSON("field/ground/patterns.json");
	auto doc = json->AsObject();
	for (auto& rule : doc["rules"]->AsObject())
	{
		auto r = rule.second->AsArray();
		rules[rule.first] = std::make_tuple(r[0]->AsString(), r[1]->AsInteger());
	}

	if (!tileModels[0])
	{
		tileModels[0] = std::make_shared<::Model>("field/ground/unit.fbx");
		tileModelKeys[0] = "_";

		int i = 1;
		for (auto& model : doc["models"]->AsObject())
		{
			tileModels[i] = std::make_shared<::Model>(fmt::format("field/ground/{}", model.second->AsString()));
			tileModelKeys[i] = model.first;
			i++;
		}
	}

	delete json;

	auto getTileElevation = [&](int x, int y)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return 4;
		return (int)Terrain[(y * Width) + x].Elevation;
	};

	/*
	Interesting idea: have each key be nine characters long.
	"?_?_-_?_?" would mean "apply this if the middle is elevated
	compared to all sides, but don't bother with the diagonals."
	"?-?_-_?-?" would mean "north and south must be the same
	elevation as the center, east and west must be lower, and
	the diagonals don't matter."
	*/

	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			auto five = getTileElevation(x, y);
			std::string key = "";
			if (getTileElevation(x - 1, y + 1) >= five) key += "1";
			if (getTileElevation(x    , y + 1) >= five) key += "2";
			if (getTileElevation(x + 1, y + 1) >= five) key += "3";
			if (getTileElevation(x - 1, y    ) >= five) key += "4";

			if (getTileElevation(x + 1, y    ) >= five) key += "6";
			if (getTileElevation(x - 1, y - 1) >= five) key += "7";
			if (getTileElevation(x    , y - 1) >= five) key += "8";
			if (getTileElevation(x + 1, y - 1) >= five) key += "9";

			if (key.empty()) //still empty and we're elevated?
			{
				if (getTileElevation(x - 1, y + 1) < five) key += "1";
				if (getTileElevation(x    , y + 1) < five) key += "2";
				if (getTileElevation(x + 1, y + 1) < five) key += "3";
				if (getTileElevation(x - 1, y    ) < five) key += "4";

				if (getTileElevation(x + 1, y    ) < five) key += "6";
				if (getTileElevation(x - 1, y - 1) < five) key += "7";
				if (getTileElevation(x    , y - 1) < five) key += "8";
				if (getTileElevation(x + 1, y - 1) < five) key += "9";

				if (key == "12346789")
					key = "#";
			}


			if (rules.find(key) != rules.end())
			{
				std::string model;
				int rotation;
				std::tie(model, rotation) = rules[key];

				TerrainModels[(y * Width) + x].Rotation = (unsigned char)rotation;

				for (int i = 0; i < tileModels.size(); i++)
				{
					if (tileModelKeys[i] == model)
					{
						TerrainModels[(y * Width) + x].Model = (unsigned char)i;
						break;
					}
				}
			}
		}
	}
}

#ifdef DEBUG
#include <stb_image_write.h>
void Map::SaveToPNG()
{
	auto pixels = new unsigned long[Width * Height * 4];
	const glm::vec3 typeColors[] =
	{
		{ 0.133, 0.545, 0.133 },
		{ 0.81, 0.59, 0.28 },
		{ 0.5, 0.5, 0.5 }
	};

	auto width = Width;
	auto set = [pixels, width](glm::vec3 color, int x, int y)
	{
		auto r = (unsigned char)(color.r * 255);
		auto g = (unsigned char)(color.g * 255);
		auto b = (unsigned char)(color.b * 255);
		auto c = (r) | (g << 8) | (b << 16) | (255 << 24);

		pixels[(y * width) + x] = c;
	};

	for (int i = 0; i < Width * Height; i++)
	{
		auto t = Terrain[i];
		auto color = typeColors[t.Type];
		color = glm::mix(color, glm::vec3(1), t.Elevation * 0.25f);
		set(color, i % Width, i / Width);
	}

	/*
	Objects on the map should have a size in tiles and a color.
	If the color's alpha is zero, the object is to be skipped.
	*/
	//set(glm::vec4(0.75), 1, 1, 5, 3);

	stbi_write_png("map.png", Width, Height, 4, pixels, Width * 4);
}
#endif


void Map::drawWorker(float dt)
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
	for (const auto& v : town->Villagers)
		v->Draw(dt * timeScale);
	MeshBucket::Flush();

	auto playerTile = glm::round(thePlayer.Position / 10.0f);
	constexpr auto half = (int)(AcreSize * 1.5f);
	constexpr auto north = AcreSize * 3;
	constexpr auto south = AcreSize + half;
	constexpr auto sides = AcreSize + half;
	const auto y1 = glm::clamp((int)playerTile.z - north, 0, Width);
	const auto y2 = glm::clamp((int)playerTile.z + south, 0, Width);
	const auto x1 = glm::clamp((int)playerTile.x - sides, 0, Height);
	const auto x2 = glm::clamp((int)playerTile.x + sides, 0, Height);
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			auto tile = Terrain[(y * Width) + x];
			auto extra = TerrainModels[(y * Width) + x];
			auto model = tileModels[extra.Model];
			auto pos = glm::vec3(x * 10, tile.Elevation * ElevationHeight, y * 10);
			auto rot = extra.Rotation * 90.0f;
			if (extra.Model == 0 || extra.Model > 44)
				model->SetLayerByMat("_mGrass", tile.Type); //ground, river, or waterfall.
			else if (extra.Model < 44)
				model->SetLayer("GrassT__mGrass", tile.Type); //cliffs should only have the top grass changed.
			model->Draw(pos, rot);
		}
	}
	MeshBucket::Flush();

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Map::Draw(float dt)
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
		groundMixs[0] = "field/ground/squares_mix.png";
		squareMix = new TextureArray(groundMixs);
	}

	if (postFx)
	{
		frameBuffer->Use();
		glClearColor(0.2f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawWorker(dt * timeScale);
		frameBuffer->Drop();
		colorMapBuffer->Use();
		frameBuffer->Draw();
		colorMapBuffer->Drop();
		colorMapBuffer->Draw();
	}
	else
	{
		colorMapBuffer->Use();
		drawWorker(dt * timeScale);
		colorMapBuffer->Drop();
		colorMapBuffer->Draw();
	}
}

bool Map::Tick(float dt)
{
	return thePlayer.Tick(dt);
}

void Map::SaveObjects(JSONObject& json)
{
	JSONArray dropsArray;
	JSONArray furnsArray;
	for (const auto& i : Objects)
	{
		if (i.Dropped)
		{
			JSONObject drop;
			drop["id"] = new JSONValue(i.Item->FullID());
			JSONArray pos;
			pos.push_back(new JSONValue((int)i.Position.x));
			pos.push_back(new JSONValue((int)i.Position.y));
			drop["position"] = new JSONValue(pos);
			dropsArray.push_back(new JSONValue(drop));
		}
		else
		{
			JSONObject furn;
			furn["id"] = new JSONValue(i.Item->FullID());
			furn["fixed"] = new JSONValue(i.Fixed != 0); //why does PVS not like "== 1"?
			furn["facing"] = new JSONValue(i.Rotation);
			furn["layer"] = new JSONValue(i.Layer);
			furn["state"] = new JSONValue(i.State);
			JSONArray pos;
			pos.push_back(new JSONValue((int)i.Position.x));
			pos.push_back(new JSONValue((int)i.Position.y));
			furn["position"] = new JSONValue(pos);
			furnsArray.push_back(new JSONValue(furn));
		}
	}
	json["drops"] = new JSONValue(dropsArray);
	json["furniture"] = new JSONValue(furnsArray);
}

Town::Town()
{
	Music = "clock";
	CanOverrideMusic = true;
	AllowRedeco = true;
	AllowTools = true;

	SquareGrass = std::rand() / ((RAND_MAX + 1u) / 100) > 75;
	weatherSeed = std::rand();

	Width = AcreSize * 1;
	Height = AcreSize * 1;
	Terrain = std::make_unique<MapTile[]>(Width * Height);
	TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);

	/*
	//TEST
	for (int i = 0; i < Width; i++)
	{
		//Top
		Terrain[i].Elevation = 1;
	}
	for (int i = 0; i < Height; i++)
	{
		//Left
		Terrain[(i * Width)].Elevation = 1;
		//Right
		Terrain[(i * Width) + (Width - 1)].Elevation = 1;
	}

	Terrain[(4 * Width) + 2].Type = 1; //single sand tile
	Terrain[(0 * Width) + 2].Type = 2; //single stone tile on cliff

	Terrain[(8 * Width) + 5].Elevation = 2; //single column
	Terrain[(7 * Width) + 4].Elevation = 1;
	Terrain[(7 * Width) + 5].Elevation = 1;
	Terrain[(7 * Width) + 6].Elevation = 1;
	Terrain[(8 * Width) + 4].Elevation = 1;
	Terrain[(8 * Width) + 6].Elevation = 1;
	Terrain[(9 * Width) + 4].Elevation = 1;
	Terrain[(9 * Width) + 5].Elevation = 1;
	Terrain[(9 * Width) + 6].Elevation = 1;

	WorkOutModels();

	//end test
	*/

	UseDrum = true;

	//GenerateNew("mappers/test.lua", 6, 6);
	Load();

#ifdef DEBUG
	SaveToPNG();
#endif
}

void Town::GenerateNew(const std::string& mapper, int width, int height)
{
	Width = AcreSize * width;
	Height = AcreSize * height;

	Terrain = std::make_unique<MapTile[]>(Width * Height);
	TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);

	auto setTile = [&](int x, int y, int type, int elevation)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return false;
		if (type < -1 || type >= 64)
			return false;
		if (elevation < -1 || elevation >= 4)
			return false;
		if (type != -1)
			Terrain[(y * Width) + x].Type = type;
		if (elevation != -1)
			Terrain[(y * Width) + x].Elevation = elevation;
		return true;
	};

	auto raise = [&](int x, int y)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return false;
		if (Terrain[(y * Width) + x].Elevation == 3)
			return false;
		Terrain[(y * Width) + x].Elevation++;
		return true;
	};

	auto elevation = [&](int x, int y)
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return -1;
		return (int)Terrain[(y * Width) + x].Elevation;
	};

	Sol["map"] = sol::new_table();
	Sol["map"]["Width"].set(Width - 1);
	Sol["map"]["Height"].set(Height - 1);
	Sol["map"]["SetTile"].set_function(setTile);
	Sol["map"]["Raise"].set_function(raise);
	Sol["map"]["Elevation"].set_function(elevation);

	Sol.script(VFS::ReadString(mapper));

	Sol["map"] = nullptr;

#ifdef DEBUG
	SaveToPNG();
#endif
}

void Town::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("map/town.json");
		auto jsonObj = json->AsObject();

		Name = jsonObj["name"]->AsString();
		weatherSeed = jsonObj["weather"]->AsInteger();
		Hemisphere = jsonObj["north"]->AsBool() ? Hemisphere::North : Hemisphere::South;
		SquareGrass = jsonObj["squareGrass"]->AsBool();

		Width = jsonObj["width"]->AsInteger();
		Height = jsonObj["height"]->AsInteger();
		Terrain = std::make_unique<MapTile[]>(Width * Height);
		TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);

		{
			auto compressedData = new unsigned char[sizeof(MapTile) * Width * Height];
			auto compressedSize = (unsigned long)base64_decode_bin(jsonObj["mapData"]->AsString(), compressedData);
			unsigned long uncompedSize;
			mz_uncompress((unsigned char*)Terrain.get(), &uncompedSize, compressedData, compressedSize);
			delete[] compressedData;
		}

		Villagers.clear();
		for (const auto& f : jsonObj["villagers"]->AsArray())
		{
			Villagers.push_back(Database::Find<Villager>(f->AsString(), villagers));
		}

		flags.clear();
		for (const auto& f : jsonObj["flags"]->AsObject())
		{
			flags[f.first] = f.second->AsInteger();
		}

		delete json;
	}
	catch (std::runtime_error&)
	{
		//Nothing to load.
		weatherSeed = 0; //Use this as a sign that there was no town data.
	}

	WorkOutModels();
}

void Town::Save()
{
	conprint(0, "Saving...");
	JSONObject json;

	auto compressedData = new unsigned char[sizeof(MapTile) * Width * Height];
	unsigned long compressedSize;
	mz_compress(compressedData, &compressedSize, (unsigned char*)Terrain.get(), sizeof(MapTile) * Width * Height);
	//BUGBUG: mz_compress may return NOTHING in Release builds!
	if (compressedSize == 0)
	{
		conprint(4, "NOT SAVING: mz_compress returned nothing!");
		return;
	}
	auto base64 = base64_encode(compressedData, compressedSize);
	json["mapData"] = new JSONValue(base64);
	delete[] compressedData;

	json["width"] = new JSONValue(Width);
	json["height"] = new JSONValue(Height);
	json["weather"] = new JSONValue((int)weatherSeed);
	json["name"] = new JSONValue(Name);
	json["north"] = new JSONValue(Hemisphere == Hemisphere::North);
	json["squareGrass"] = new JSONValue(SquareGrass);

	JSONArray villagersArray;
	for (const auto& i : Villagers)
	{
		villagersArray.push_back(new JSONValue(i->ID));
	}
	json["villagers"] = new JSONValue(villagersArray);

	SaveObjects(json);

	JSONObject flagsObj;
	for (const auto& i : flags)
	{
		flagsObj[i.first] = new JSONValue(i.second);
	}
	json["flags"] = new JSONValue(flagsObj);
	
	auto val = JSONValue(json);
	VFS::WriteSaveJSON("map/town.json", &val);
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
			const auto until = GetJSONVec2(calendar[i]->AsObject().at("until"));
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

void Town::Draw(float dt)
{
	Map::Draw(dt);

	if ((int)Clouds >= (int)Town::Weather::RainClouds)
		rainLayer->Draw(dt * timeScale);

	Sprite::FlushBatch();
}
