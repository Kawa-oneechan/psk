#include <ctime>
#include "engine/Model.h"
#include "engine/JSONUtils.h"
#include "engine/Random.h"
#include "engine/Framebuffer.h"
#include "engine/Console.h"
#include "Types.h"
#include "Map.h"
#include "Game.h"
#include "Background.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Player.h"
#include "Camera.h"
#include "Utilities.h"

static std::array<ModelP, 80> tileModels;
static std::array<std::string, 80> tileModelKeys;

float lastGrassColor = 100.0f;

TextureArray* groundTextureAlbs{ nullptr };
TextureArray* groundTextureNrms{ nullptr };
TextureArray* groundTextureMixs{ nullptr };
TextureArray* grassColors{ nullptr };

std::shared_ptr<TextureArray> cloudImage;
std::shared_ptr<Texture> starsImage, skyImage;

void UpdateTileModelTextures()
{
	for (const auto& model : tileModels)
	{
		if (!model)
			continue;

		for (auto& mesh : model->Meshes)
		{
			if (mesh.Name.find("mGrass") != std::string::npos)
			{
				if (mesh.Name.find("mGrassCliffXlu") != std::string::npos)
					continue;
				mesh.Textures[0] = groundTextureAlbs;
				mesh.Textures[1] = groundTextureNrms;
				mesh.Textures[2] = groundTextureMixs;
				mesh.Textures[3] = grassColors;
			}
		}
	}
}

static void UpdateGrass()
{
	if (glm::abs(lastGrassColor - commonUniforms.GrassColor) < glm::epsilon<float>())
		return;

	lastGrassColor = commonUniforms.GrassColor;
	//if (commonUniforms.GrassColor <= 0.052f || commonUniforms.GrassColor >= 0.865f)
	{
		delete[] groundTextureAlbs;
		groundTextureAlbs = nullptr;
	}
}

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
//extern Background* rainLayer;
extern Framebuffer* postFxBuffer;

extern unsigned int commonBuffer;

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

void Map::WorkOutModels()
{
	std::map<std::string, std::tuple<std::string, int>> rules;

	auto json = VFS::ReadJSON("field/ground/patterns.json").as_object();
	for (auto& rule : json["rules"].as_object())
	{
		auto r = rule.second.as_array();
		rules[rule.first] = std::make_tuple(r[0].as_string(), r[1].as_integer());
	}

	if (!tileModels[0])
	{
		tileModels[0] = std::make_shared<::Model>("field/ground/unit.fbx");
		tileModelKeys[0] = "_";

		int i = 1;
		for (auto& model : json["models"].as_object())
		{
			tileModels[i] = std::make_shared<::Model>(fmt::format("field/ground/{}", model.second.as_string()));
			tileModelKeys[i] = model.first;
			i++;
		}
	}

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
			if (getTileElevation(x, y + 1) >= five) key += "2";
			if (getTileElevation(x + 1, y + 1) >= five) key += "3";
			if (getTileElevation(x - 1, y) >= five) key += "4";

			if (getTileElevation(x + 1, y) >= five) key += "6";
			if (getTileElevation(x - 1, y - 1) >= five) key += "7";
			if (getTileElevation(x, y - 1) >= five) key += "8";
			if (getTileElevation(x + 1, y - 1) >= five) key += "9";

			if (key.empty()) //still empty and we're elevated?
			{
				if (getTileElevation(x - 1, y + 1) < five) key += "1";
				if (getTileElevation(x, y + 1) < five) key += "2";
				if (getTileElevation(x + 1, y + 1) < five) key += "3";
				if (getTileElevation(x - 1, y) < five) key += "4";

				if (getTileElevation(x + 1, y) < five) key += "6";
				if (getTileElevation(x - 1, y - 1) < five) key += "7";
				if (getTileElevation(x, y - 1) < five) key += "8";
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

void Map::drawCharacters(float dt)
{
	for (const auto& p : People)
		p->Draw(dt * timeScale);
	MeshBucket::Flush();
}

void Map::drawObjects(float dt)
{
	(void)(dt);

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
			i.Item->DrawFieldModel(pos3, rotation);
		}
	}
	MeshBucket::Flush();
}

void Map::drawGround(float dt)
{
	(void)(dt);

	auto playerTile = glm::round(thePlayer.Position / 10.0f);

	constexpr auto half = (int)(AcreSize * 1.5f);
	constexpr auto north = AcreSize * 3;
	constexpr auto south = AcreSize + half;
	constexpr auto sides = AcreSize + half;
	const auto y1 = glm::clamp((int)playerTile.z - north, 0, Width);
	const auto y2 = glm::clamp((int)playerTile.z + south, 0, Width);
	const auto x1 = glm::clamp((int)playerTile.x - sides, 0, Height);
	const auto x2 = glm::clamp((int)playerTile.x + sides, 0, Height);

	const auto cullMargin = 192;

	glm::vec2 proj;
	glm::vec3 pos;

	auto draw = [&](int x, int y, glm::vec3& pos) -> bool
	{
		if (x < 0 || y < 0 || x >= Width || y >= Height)
			return true;
		Project(pos, proj);
		if (proj.x < -cullMargin || proj.x > width + cullMargin)
			return false;
		auto index = (y * Width) + x;
		auto tile = Terrain[index];
		if (tile.Type == TileType::Special)
			return true;
		pos.y = (float)tile.Elevation * ElevationHeight;
		auto extra = TerrainModels[(y * Width) + x];
		auto model = tileModels[extra.Model];
		auto rot = extra.Rotation * 90.0f;
		if (extra.Model == 0 || extra.Model > 44)
			model->SetLayerByMat("mGrass", tile.Type); //ground, river, or waterfall.
		else if (extra.Model < 44)
			model->SetLayer("GrassT__mGrass", tile.Type); //cliffs should only have the top grass changed.
		
		model->Bones[0].Translation = pos;
		model->Bones[0].Rotation = glm::vec3(0, glm::radians(rot), 0);;
		model->CalculateBoneTransforms();
		model->Draw();
		//model->Draw(pos, rot);
		return true;
	};

	for (int y = y1; y < y2; y++)
	{
		pos.z = y * 10.0f;
		for (int x = (int)playerTile.x - 1; x >= x1; x--)
		{
			pos.x = x * 10.0f;
			pos.y = 0.0f;
			if (!draw(x, y, pos))
				break;
		}
		for (int x = (int)playerTile.x; x < x2; x++)
		{
			pos.x = x * 10.0f;
			pos.y = 0.0f;
			if (!draw(x, y, pos))
				break;
		}
	}
	MeshBucket::Flush();
	MeshBucket::FlushTranslucent();
}

void Map::drawWorker(float dt)
{
	UpdateGrass();

	if (wireframe)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	MeshBucket::DrawAllWithDepth(dt, [&, dt] { drawCharacters(dt); drawObjects(dt); drawGround(dt); });

	if (!wireframe)
	{
		glEnable(GL_DEPTH_TEST);
		cloudImage->Use(1);
		starsImage->Use(2);
		skyImage->Use(3);

		Sprite::DrawSprite(Shaders["sky"], *whiteRect, glm::vec2(0), glm::vec2(width, height));
		Sprite::FlushBatch();
		glDisable(GL_DEPTH_TEST);
	}

	//For an interior map:
	/*
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	MeshBucket::DrawAllWithDepth(dt, [&, dt] {
		drawCharacters(dt);
		drawObjects(dt);
		drawRoom(dt); OR drawInterior(dt); I dunno
	});
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
	tm gm{};
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	commonUniforms.TimeOfDay = (gm.tm_hour / 24.0f) + ((gm.tm_min / 60.0f) / 24.0f);

	for (const auto& p : People)
	{
		if (!p->Tick(dt))
			return false;
	}

	return true;
}

void Map::SaveObjects(jsonValue& json)
{
	auto objects = json5pp::array({});
	for (const auto& a : Acres)
	{
		for (const auto& i : a.Objects)
		{
			auto item = json5pp::object({});
			item.as_object()["id"] = i.Item->FullID();
			item.as_object()["position"] = GetJSONVecI(i.Position / 10.0f);
			if (i.State != 0)
			{
				item.as_object()["state"] = i.State;
			}
			if (i.Dropped)
			{
				item.as_object()["dropped"] = true;
			}
			else
			{
				if (i.Fixed)
					item.as_object()["fixed"] = true;
				if (i.Rotation != 0)
					item.as_object()["facing"] = i.Rotation;
				if (i.Layer != ItemLayer::Ground)
					item.as_object()["layer"] = (int)i.Layer;
			}
			objects.as_array().push_back(item);
		}
	}
	json.as_object()["objects"] = std::move(objects);
}

void Map::LoadObjects(jsonValue& json)
{
	auto doc = json.as_object();
	auto objects = doc["objects"].is_array() ? doc["objects"].as_array() : json5pp::array({}).as_array();

	for (auto& _i : objects)
	{
		auto i = _i.as_object();
		MapItem item;
		item.Item = std::make_shared<InventoryItem>(i["id"].as_string());
		item.Position = GetJSONVec2(i["position"]) * 10.0f;
		item.State = i["state"].is_number() ? (int)i["state"].as_number() : 0;
		item.Dropped = i["dropped"].is_boolean() ? i["dropped"].as_boolean() : false;
		if (item.Dropped)
		{
			item.Fixed = false;
			item.Layer = ItemLayer::Ground;
			item.Rotation = 0;
		}
		else
		{
			item.Fixed = i["fixed"].is_boolean() ? i["fixed"].as_boolean() : false;
			item.Layer = (ItemLayer)(i["layer"].is_integer() ? i["layer"].as_integer() : 0);
			item.Rotation = i["facing"].is_integer() ? i["facing"].as_integer() : 0;
		}
		
		int acreX = (int)(item.Position.x / 10.0f) / AcreSize;
		int acreY = (int)(item.Position.y / 10.0f) / AcreSize;
		auto acreIndex = (acreY * (Width / AcreSize)) + acreX;
		if (acreIndex < 0 || acreIndex >= Acres.size())
		{
			conprint(4, "Item \"{}\" at {}x{} is out of range.", item.Item->FullID(), item.Position.x, item.Position.y);
			continue;
		}
		Acres[acreIndex].Objects.push_back(item);
	}

	//TODO: go through the list of objects later on to correct wrong layers.
	//In exteriors, items only go on the ground or on tables.
	//Items on the table layer should have a supporter on the ground layer.
}
