#include "Tilemap.h"
#include "TextUtils.h"
#include "VFS.h"
#include "SpriteRenderer.h"
#include "../Game.h"

extern float scale;
extern "C" unsigned char* unbase64(const char* ascii, int len, int *flen);

Tilemap::MapLayer::MapLayer(jsonValue& doc, Tilemap* owner) : owner(owner)
{
	auto lo = doc.as_object();

	ID = lo["name"].as_string();
	width = lo["width"].as_integer();
	height = lo["height"].as_integer();

	Parallax = glm::vec2(1);
	if (lo["parallaxx"].is_number())
		Parallax.x = lo["parallaxx"].as_number();
	if (lo["parallaxy"].is_number())
		Parallax.y = lo["parallaxy"].as_number();

	if (lo["tintcolor"].is_string())
		Tint = GetJSONColor(lo["tintcolor"]);
	if (lo["opacity"].is_number())
		Tint.a *= lo["opacity"].as_number();

	auto totalSize = this->width * this->height;

	data = std::make_unique<unsigned int[]>(totalSize);
	if (lo["data"].is_array())
	{
		int i = 0;
		for (const auto& di : lo["data"].as_array())
			data[i++] = (unsigned int)di.as_integer();
	}
	else if (lo["data"].is_string())
	{
		//Assume base64, no compression.
		auto b64 = lo["data"].as_string();
		int len = 0;
		auto result = unbase64(b64.c_str(), (int)b64.length(), &len);
		if (len != totalSize * sizeof(unsigned int))
			throw std::runtime_error("Oh SHIT");
		std::memcpy(data.get(), result, len);
		free(result);
	}
}

void Tilemap::MapLayer::Draw(float dt)
{
	(void)(dt);
	if (!data)
		return;

	float s = Scale > 0 ? Scale : scale;

	auto cam = Camera * Parallax;

	const auto& tiles = owner->tileset;

	auto top = (int)(cam.y / tiles.tileGridHeight);
	if (top < 0) top = 0;
	auto bottom = top + (int)((BECKETT_SCREENHEIGHT / tiles.tileGridHeight) / s) + 1;
	if (bottom > height) bottom = height;
	auto left = (int)(cam.x / tiles.tileGridWidth);
	if (left < 0) left = 0;
	auto right = left + (int)((BECKETT_SCREENWIDTH / tiles.tileGridWidth) / s) + 1;
	if (right > width) right = width;

	const auto tileSize = glm::vec2(tiles.tileWidth, tiles.tileHeight) * s;
	for (auto row = top; row < bottom; row++)
	{
		for (auto col = left; col < right; col++)
		{
			auto tile = data[row * width + col];

			if (tile == 0)
				continue;

			auto flips = (tile >> 28) & 0x0F;
			tile &= 0x00FF'FFFF;
			tile--;

			auto anim = tiles.animations.find(tile);
			if (anim != tiles.animations.end())
				tile = anim->second.tile[anim->second.currentFrame];

			auto srcX = (tile % tiles.tilesPerLine) * tiles.tileWidth;
			auto srcY = (tile / tiles.tilesPerLine) * tiles.tileHeight;
			auto srcRect = glm::vec4(srcX, srcY, tiles.tileWidth, tiles.tileHeight);

			auto dest = !owner->isometric ?
				glm::vec2(col * tiles.tileWidth, row * tiles.tileHeight) :
				glm::vec2(
				((col - row) * (tiles.tileGridWidth / 2)) - (tiles.tileWidth / 2),
					((row + col) * (tiles.tileGridHeight / 2))
				);

			auto flags = (int)Sprite::SpriteFlags::RotateCenter;
			auto angle = 0.0f;
			if ((flips & 0x08) == 0x08)
				flags |= Sprite::SpriteFlags::FlipX;
			if ((flips & 0x04) == 0x04)
				flags |= Sprite::SpriteFlags::FlipY;
			if ((flips & 0x02) == 0x02)
				angle = 90.0f;

			Sprite::DrawSprite(*tiles.texture, ((dest - cam - tiles.tileOffset) * s) + owner->Position, tileSize, srcRect, angle, Tint, (Sprite::SpriteFlags)flags);
		}
	}
}

void Tilemap::MapLayer::SetTile(int row, int col, int tile)
{
	col = glm::clamp(col, 0, width - 1);
	row = glm::clamp(row, 0, height - 1);
	data[(row * width) + col] = tile + 1;
}

const int Tilemap::MapLayer::GetTile(int row, int col) const
{
	col = glm::clamp(col, 0, width - 1);
	row = glm::clamp(row, 0, height - 1);
	return data[(row * width) + col] - 1;
}

const int Tilemap::MapLayer::GetTile(const glm::vec2& position) const
{
	auto pos = position / Scale;
	pos += Camera;
	pos.x /= owner->tileset.tileWidth;
	pos.y /= owner->tileset.tileHeight;
	return GetTile((int)pos.y, (int)pos.x);
}

glm::vec2 Tilemap::MapLayer::GetPixelSize()
{
	auto ret = glm::vec2(width * owner->tileset.tileGridWidth, height * owner->tileset.tileGridHeight);
	return ret;
}

glm::vec2 Tilemap::MapLayer::GetTileSize()
{
	auto ret = glm::vec2(width, height);
	return ret;
}

glm::vec4 Tilemap::MapLayer::GetCollision(int row, int col)
{
	if (col < 0 || col >= width || row < 0 || row >= height)
		return glm::vec4(0, 0, owner->tileset.tileWidth, owner->tileset.tileHeight);
	col = glm::clamp(col, 0, width - 1);
	row = glm::clamp(row, 0, height - 1);
	auto tile = owner->tileset.collisions.find(data[(row * width) + col] - 1);
	if (tile != owner->tileset.collisions.end())
		return tile->second.rectP;
	return glm::vec4(-1);
}

Tilemap::Tilemap(const std::string& source)
{
	auto doc = VFS::ReadJSON(source);
	auto obj = doc.as_object();

	glm::vec2 tileSize;

	std::string here;
	{
		auto slashPos = source.rfind('/');
		if (slashPos != std::string::npos)
			here = source.substr(0, slashPos + 1);
	}
	auto ts = obj["tilesets"].as_array()[0].as_object();
	if (ts["source"].is_string())
	{
		auto tsp = here + ts["source"].as_string();
		tsp = ResolvePath(tsp);
		ts = VFS::ReadJSON(tsp).as_object();
		auto slashPos = tsp.rfind('/');
		if (slashPos != std::string::npos)
			here = tsp.substr(0, slashPos + 1);
	}
	{
		tileset.tileWidth = ts["tilewidth"].as_integer();
		tileset.tileHeight = ts["tileheight"].as_integer();

		tileset.tileGridWidth = tileset.tileWidth;
		tileset.tileGridHeight = tileset.tileHeight;

		tileSize = glm::vec2(tileset.tileWidth, tileset.tileHeight);

		auto& src = ts["image"].as_string();
		auto tsp = here + src;
		tsp = ResolvePath(tsp);
		tileset.texture = VFS::GetTexture(tsp, GL_CLAMP, 0, true);

		tileset.tilesPerLine = tileset.texture->width / tileset.tileWidth;

		if (isometric)
		{
			auto grid = ts["grid"].as_object();
			tileset.tileGridWidth = grid["width"].as_integer();
			tileset.tileGridHeight = grid["height"].as_integer();
			auto offset = ts["tileoffset"].as_object();
			tileset.tileOffset = glm::vec2(offset["x"].as_integer(), offset["y"].as_integer());
		}

		if (ts["tiles"].is_array())
		{
			for (auto t : ts["tiles"].as_array())
			{
				auto id = t.as_object()["id"].as_integer();
				if (t.as_object()["animation"].is_array())
				{
					auto anim = t.as_object()["animation"].as_array();
					auto& newAnim = tileset.animations[id];
					newAnim.frameCt = glm::clamp((int)anim.size(), 0, MaxAnimFrames);
					for (int f = 0; f < newAnim.frameCt; f++)
					{
						newAnim.tile[f] = anim[f].as_object()["tileid"].as_integer();
						newAnim.duration[f] = anim[f].as_object()["duration"].as_integer();
					}
					newAnim.durationLeft = newAnim.duration[0];
				}
				if (t.as_object()["objectgroup"].is_object())
				{
					//only one object per tile for now please
					auto ob = t.as_object()["objectgroup"].as_object()["objects"].as_array()[0].as_object();
					auto& newObj = tileset.collisions[id];
					newObj.name = ob["name"].as_string();
					newObj.type = ob["type"].as_string();
					auto pos = glm::vec2(ob["x"].as_number(), ob["y"].as_number());
					auto size = glm::vec2(ob["width"].as_number(), ob["height"].as_number());
					newObj.rectP = glm::vec4(pos, pos + size);
					newObj.rectT = glm::vec4(pos / tileSize, (pos + size) / tileSize);
				}
			}
		}
	}

	for (auto& l : obj["layers"].as_array())
	{
		auto lo = l.as_object();
		if (lo["type"].as_string() == "tilelayer")
			ChildTickables.push_back(std::make_shared<MapLayer>(l, this));
		if (lo["type"].as_string() == "objectgroup")
		{
			if (lo["name"].as_string() == "Sprites")
			{
				ChildTickables.push_back(std::make_shared<MapSpriteLayer>());
				continue;
			}

			for (auto ob : lo["objects"].as_array())
			{
				auto o = ob.as_object();
				auto pos = glm::vec2(o["x"].as_number(), o["y"].as_number());
				auto size = glm::vec2(o["width"].as_number(), o["height"].as_number());
				Shape shp = {
					glm::vec4(pos, pos + size),
					glm::vec4(pos / tileSize, (pos + size) / tileSize),
					o["name"].as_string(),
					o["type"].as_string()
				};
				shapes.push_back(shp);
			}
		}
	}

	Camera = glm::vec2(0);
}

bool Tilemap::Tick(float dt)
{
	for (auto& a : tileset.animations)
	{
		a.second.durationLeft -= (int)(dt * 1000);
		if (a.second.durationLeft <= 0)
		{
			a.second.currentFrame++;
			if (a.second.currentFrame >= a.second.frameCt)
				a.second.currentFrame = 0;
			a.second.durationLeft = a.second.duration[a.second.currentFrame];
		}
	}

	for (const auto& l : ChildTickables)
	{
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(l);
		if (mapLayer)
		{
			mapLayer->Scale = Scale;
			mapLayer->Camera = Camera;
			continue;
		}
		auto spriteLayer = std::dynamic_pointer_cast<MapSpriteLayer>(l);
		if (spriteLayer)
		{
			spriteLayer->Scale = Scale;
			spriteLayer->Position = -Camera;
			continue;
		}
	}

	return Tickable2D::Tick(dt);
}

void Tilemap::Draw(float dt)
{
	Tickable2D::Draw(dt);
}

std::shared_ptr<Tickable> Tilemap::GetLayer(size_t i)
{
	if (i < ChildTickables.size()) return ChildTickables[i];
	return ChildTickables[0];
}

void Tilemap::SetTile(int row, int col, int tile)
{
	for (auto& l : ChildTickables)
	{
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(l);
		if (mapLayer)
			mapLayer->SetTile(row, col, tile);
	}
}

void Tilemap::SetTile(int row, int col, std::initializer_list<int> tiles)
{
	for (int i = 0; i < ChildTickables.size() && i < tiles.size(); i++)
	{
		auto t = *(tiles.begin() + i);
		if (t == -2)
			continue;
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(ChildTickables[i]);
		if (mapLayer)
		{
			mapLayer->SetTile(row, col, t);
		}
	}
}

const int Tilemap::GetTile(int layer, int row, int col) const
{
	auto mapLayer = std::dynamic_pointer_cast<MapLayer>(ChildTickables[layer]);
	if (mapLayer)
		return mapLayer->GetTile(row, col);
	return -1;
}

const int Tilemap::GetTile(int layer, const glm::vec2& position) const
{
	auto mapLayer = std::dynamic_pointer_cast<MapLayer>(ChildTickables[layer]);
	if (mapLayer)
		return mapLayer->GetTile(position);
	return -1;
}

glm::vec2 Tilemap::GetPixelSize()
{
	for (auto& l : ChildTickables)
	{
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(l);
		if (mapLayer)
		{
			auto ret = mapLayer->GetPixelSize();
			return ret;
		}
	}
	return glm::vec2(16);
}

glm::vec2 Tilemap::GetTileSize()
{
	for (auto& l : ChildTickables)
	{
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(l);
		if (mapLayer)
		{
			auto ret = mapLayer->GetTileSize();
			return ret;
		}
	}
	return glm::vec2(16);
}

glm::vec4 Tilemap::GetCollision(int row, int col)
{
	for (auto& l : ChildTickables)
	{
		auto mapLayer = std::dynamic_pointer_cast<MapLayer>(l);
		if (mapLayer)
		{
			auto ret = mapLayer->GetCollision(row, col);
			if (ret.x != -1)
				return ret;
		}
	}
	return glm::vec4(-1);
}

bool MapSpriteLayer::Tick(float dt)
{
	Tickable2D::Tick(dt);

	for (const auto& s : ChildTickables)
	{
		auto ms = std::dynamic_pointer_cast<MapSprite>(s);
		if (ms)
		{
			ms->Scale = Scale;
		}
	}
	return true;
}

void MapSpriteLayer::Draw(float dt)
{
	std::vector<Tickable2D*> sorted;
	sorted.reserve(ChildTickables.size());
	for (const auto& l : ChildTickables)
	{
		auto t2D = std::dynamic_pointer_cast<Tickable2D>(l);
		if (t2D)
			sorted.push_back(t2D.get());
	}
	std::sort(sorted.begin(), sorted.end(), [](const Tickable2D* a, const Tickable2D* b)
	{
		return a->Position.y < b->Position.y;
	});
	for (auto l : sorted)
	{
		l->Draw(dt);
	}
}
