#include "Town.h"
#include "Framebuffer.h"
#include "Background.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Model.h"

static std::array<ModelP, 80> tileModels;
static std::array<std::string, 80> tileModelKeys;

float Map::GetHeight(const glm::vec3& pos)
{
	/*
	TODO: check if there's a building, special acre, or placed object here.
	If so, use proper ray casting to find out the answer.
	*/
	int tx = (int)glm::round(pos.x / 10.0f);
	int ty = (int)glm::round(pos.z / 10.0f);
	auto tile = Terrain[tx + (ty * Width)];
	return (float)tile.Elevation * ElevationHeight;
}

float Map::GetHeight(int x, int y)
{
	return GetHeight(glm::vec3(x, 100, y));
}

extern float timeScale;
extern bool wireframe;
extern Shader* modelShader;
extern Shader* skyShader;
extern Background* rainLayer;
extern Framebuffer* postFxBuffer;

extern unsigned int commonBuffer;

float lastGrassColor = 100.0f;

TextureArray* groundTextureAlbs{ nullptr };
TextureArray* groundTextureNrms{ nullptr };
TextureArray* groundTextureMixs{ nullptr };
TextureArray* grassColors{ nullptr };

static void UpdateGrass()
{
	if (glm::abs(lastGrassColor - commonUniforms.GrassColor) < glm::epsilon<float>())
		return;

	lastGrassColor = commonUniforms.GrassColor;

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
				mesh.Textures[2] = groundTextureMixs;
				mesh.Textures[3] = grassColors;
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

#ifdef DEBUG
extern bool Project(const glm::vec3& in, glm::vec2& out);
#endif

void Map::drawCharacters(float dt)
{
	thePlayer.Draw(dt * timeScale);
	MeshBucket::Flush();
	for (const auto& v : town->Villagers)
		v->Draw(dt * timeScale);
	MeshBucket::Flush();
}

void Map::drawObjects(float dt)
{
	auto playerTile = glm::round(thePlayer.Position / 10.0f);

	auto playerAcre = playerTile / (float)AcreSize;
	auto playerAcreIdx = ((int)playerAcre.y * (Width / AcreSize)) * (int)playerAcre.y;
	//acrePos needs to start in alignment and increase by a lot.
	auto acrePos = glm::vec3(0, 0, 0); //-V821 yeah I know, this is for later bby
	//TODO: draw surrounding acres too.
	/*
	Suggest building an array of nine offsets:
		-n - 1, -n,  -n + 1,
		   - 1,  0,     + 1,
		 n - 1,  n,   n + 1
	where n = (Width / AcreSize), find playerAcreIdx,
	then loop through this list. If out of bounds, skip.
	*/
	{
		auto acreModel = Acres[playerAcreIdx].Model;
		if (acreModel)
			acreModel->Draw(acrePos);

		for (auto& i : Acres[playerAcreIdx].Objects)
		{
			if (!i.Dropped) continue;
			auto height = GetHeight(glm::vec3(i.Position.x, 100, i.Position.y));
			auto pos3 = glm::vec3(i.Position.x, height, i.Position.y);
			i.Item->Wrapped()->DrawFieldIcon(pos3);
		}

		for (auto& i : Acres[playerAcreIdx].Objects)
		{
			if (i.Dropped) continue;
			auto height = GetHeight(glm::vec3(i.Position.x, 100, i.Position.y));
			//TODO: handle layers
			auto pos3 = glm::vec3(i.Position.x, height, i.Position.y);
			auto rotation = i.Rotation * 90.0f;
			i.Item->Wrapped()->DrawFieldModel(pos3, rotation);
		}
	}
	MeshBucket::Flush();
}

void Map::drawGround(float dt)
{
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
			if (tile.Type == TileType::Special)
				continue;
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
}

void Map::drawWorker(float dt)
{
	Sprite::DrawSprite(skyShader, *whiteRect, glm::vec2(0), glm::vec2(width, height));
	Sprite::FlushBatch();

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

	UpdateGrass();

	drawCharacters(dt);
	drawObjects(dt);
	drawGround(dt);

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//For an interior map:
	/*
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	drawCharacters(dt);
	drawObjects(dt);
	drawRoom(dt); OR drawInterior(dt); I dunno
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	*/
}

void Map::Draw(float dt)
{
	postFxBuffer->Use();
	drawWorker(dt * timeScale);
	postFxBuffer->Drop();
	postFxBuffer->Draw();
}

bool Map::Tick(float dt)
{
	//TODO: perhaps put this in a more sensible place.
	//TODO: allow non-walltime
	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	commonUniforms.TimeOfDay = (gm.tm_hour / 24.0f) + ((gm.tm_min / 60.0f) / 24.0f);

	return thePlayer.Tick(dt);
}

void Map::SaveObjects(JSONObject& json)
{
	JSONArray dropsArray;
	JSONArray furnsArray;
	for (const auto& a : Acres)
	{
		for (const auto& i : a.Objects)
		{
			if (i.Dropped)
			{
				JSONObject drop;
				drop["id"] = new JSONValue(i.Item->FullID());
				drop["position"] = GetJSONVec(i.Position / 10.0f, true);
				if (i.State != 0)
					drop["state"] = new JSONValue(i.State);
				dropsArray.push_back(new JSONValue(drop));
			}
			else
			{
				JSONObject furn;
				furn["id"] = new JSONValue(i.Item->FullID());
				furn["fixed"] = new JSONValue(i.Fixed);
				furn["facing"] = new JSONValue(i.Rotation);
				furn["layer"] = new JSONValue(i.Layer);
				if (i.State != 0)
					furn["state"] = new JSONValue(i.State);
				furn["position"] = GetJSONVec(i.Position / 10.0f, true);
				furnsArray.push_back(new JSONValue(furn));
			}
		}
	}
	json["drops"] = new JSONValue(dropsArray);
	json["furniture"] = new JSONValue(furnsArray);
}

void Map::LoadObjects(JSONObject& json)
{
	auto dropsArray = json["drops"] != nullptr ? json["drops"]->AsArray() : JSONArray();
	auto furnsArray = json["furniture"] != nullptr ? json["furniture"]->AsArray() : JSONArray();

	for (auto& _i : dropsArray)
	{
		auto i = _i->AsObject();
		MapItem drop;
		drop.Item = std::make_shared<InventoryItem>(i["id"]->AsString());
		drop.Dropped = true;
		drop.Position = GetJSONVec2(i["position"]) * 10.0f;
		drop.Fixed = false;
		drop.Layer = 0;
		drop.Rotation = 0;
		drop.State = (i["state"] != nullptr) ? i["state"]->AsInteger() : 0;
		
		int acreX = (int)(drop.Position.x / 10.0f) / AcreSize;
		int acreY = (int)(drop.Position.y / 10.0f) / AcreSize;
		auto acreIndex = (acreY * (Width / AcreSize)) + acreX;
		if (acreIndex < 0 || acreIndex >= Acres.size())
		{
			conprint(4, "Item \"{}\" at {}x{} is out of range.", drop.Item->FullID(), drop.Position.x, drop.Position.y);
			continue;
		}
		Acres[acreIndex].Objects.push_back(drop);
	}

	for (auto& _i : furnsArray)
	{
		auto i = _i->AsObject();
		MapItem furn;
		furn.Item = std::make_shared<InventoryItem>(i["id"]->AsString());
		furn.Dropped = false;
		furn.Position = GetJSONVec2(i["position"]) * 10.0f;
		furn.Fixed = (i["fixed"] != nullptr) ? i["fixed"]->AsBool() : false;
		furn.Layer = (i["layer"] != nullptr) ? i["layer"]->AsInteger() : 0;
		furn.Rotation = (i["facing"] != nullptr) ? i["facing"]->AsInteger() : 0;
		furn.State = (i["state"] != nullptr) ? i["state"]->AsInteger() : 0;

		int acreX = (int)(furn.Position.x / 10.0f) / AcreSize;
		int acreY = (int)(furn.Position.y / 10.0f) / AcreSize;
		auto acreIndex = (acreY * (Width / AcreSize)) + acreX;
		if (acreIndex < 0 || acreIndex >= Acres.size())
		{
			conprint(4, "Item \"{}\" at {}x{} is out of range.", furn.Item->FullID(), furn.Position.x, furn.Position.y);
			continue;
		}
		Acres[acreIndex].Objects.push_back(furn);
	}
}

Town::Town()
{
	Music = "clock";
	CanOverrideMusic = true;
	AllowRedeco = true;
	AllowTools = true;

	grassColorMap = "grasscolors.png";
	grassTexture = "design0_mix.png";
	if (rnd::getFloat() > 0.75f)
		grassTexture = "squares_mix.png";
	grassCanSnow = true;
	weatherSeed = rnd::getInt();

	Width = AcreSize * 1;
	Height = AcreSize * 1;
	Terrain = std::make_unique<MapTile[]>(Width * Height);
	TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);
	Acres.clear();
	Acres.resize((Width / AcreSize) * (Height / AcreSize));

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
	Acres.clear();
	Acres.resize(width * height);

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
		grassCanSnow = jsonObj["grassCanSnow"]->AsBool();
		grassColorMap = jsonObj["grassColors"]->AsString();
		grassTexture = jsonObj["grassTexture"]->AsString();

		Width = jsonObj["width"]->AsInteger();
		Height = jsonObj["height"]->AsInteger();
		Terrain = std::make_unique<MapTile[]>(Width * Height);
		TerrainModels = std::make_unique<ExtraTile[]>(Width * Height);
		
		Acres.clear();
		Acres.resize((Width / AcreSize) * (Height / AcreSize));
		auto acres = jsonObj["acres"]->AsArray();
		for (int i = 0; i < acres.size() && i < Acres.size(); i++)
		{
			if (acres[i]->IsNull())
				Acres[i].Model.reset();
			else
			{
				Acres[i].Model = std::make_shared<::Model>(fmt::format("field/acres/{}.fbx", acres[i]->AsString()));
				Acres[i].ModelName = acres[i]->AsString();
			}
		}

		VFS::ReadSaveData(Terrain.get(), "map/map.bin");

		Villagers.clear();
		for (const auto& f : jsonObj["villagers"]->AsArray())
		{
			Villagers.push_back(Database::Find<Villager>(f->AsString(), villagers));
		}

		LoadObjects(jsonObj);

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

	VFS::WriteSaveData("map/map.bin", (void*)Terrain.get(), sizeof(MapTile) * Width * Height);

	json["width"] = new JSONValue(Width);
	json["height"] = new JSONValue(Height);
	json["weather"] = new JSONValue((int)weatherSeed);
	json["name"] = new JSONValue(Name);
	json["north"] = new JSONValue(Hemisphere == Hemisphere::North);

	json["grassTexture"] = new JSONValue(grassTexture);
	json["grassCanSnow"] = new JSONValue(grassCanSnow);
	json["grassColors"] = new JSONValue(grassColorMap);

	JSONArray villagersArray;
	for (const auto& i : Villagers)
	{
		villagersArray.push_back(new JSONValue(i->ID));
	}
	json["villagers"] = new JSONValue(villagersArray);

	JSONArray acresArray;
	for (const auto& i : Acres)
	{
		if (i.Model)
			acresArray.push_back(new JSONValue(i.ModelName));
		else
			acresArray.push_back(new JSONValue());
	}
	json["acres"] = new JSONValue(acresArray);

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
		//std::srand(weatherSeed + calNow);
		std::random_device device;
		std::mt19937 engine(device());
		engine.seed(weatherSeed + calNow);

		std::vector<int> rates;
		int rateTotal = 0;
		for (auto r : here["rates"]->AsArray())
		{
			rateTotal += r->AsInteger();
			rates.push_back(r->AsInteger());
		}
		std::uniform_int_distribution<> dist(0, rateTotal);
		auto roll = dist(engine);
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

	//Reset textures so snow can appear/disappear
	delete[] groundTextureAlbs;
	groundTextureAlbs = nullptr;
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
		auto windStrength = ((1 << baseWind) - 1) + rnd::getInt(3);
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

		if (grassCanSnow && commonUniforms.GrassColor <= 0.052f || commonUniforms.GrassColor >= 0.865f)
			groundMixs[0] = "field/ground/snow_mix.png";
		else
			groundMixs[0] = fmt::format("field/ground/{}", grassTexture);

		groundTextureAlbs = new TextureArray(groundAlbs);
		groundTextureNrms = new TextureArray(groundNrms);
		groundTextureMixs = new TextureArray(groundMixs);
		grassColors = new TextureArray(fmt::format("field/ground/{}", grassColorMap), GL_CLAMP_TO_EDGE, GL_NEAREST);
	}

	Map::Draw(dt);

	if ((int)Clouds >= (int)Town::Weather::RainClouds)
		rainLayer->Draw(dt * timeScale);

	Sprite::FlushBatch();
}
