#include <algorithm>
#include "PanelLayout.h"
#include "engine/InputsMap.h"
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Console.h"
#include "engine/SpriteRenderer.h"
#include "engine/ShapeUtils.h"
#include "Database.h"
#include "Types.h"
#include "Game.h"
#include "Utilities.h"

bool debugPanelLayoutPolygons = false;
bool debugRenderPanelLayouts = true;

PanelLayout::PanelLayout(jsonValue& source)
{
	auto src = source.as_object();

	auto const& panelsSet = src["panels"].as_array();

	Position = src["position"].is_array() ? GetJSONVec2(src["position"]) : glm::vec2(0);
	Alpha = GetJSONVal(src["alpha"], 1.0f);
	Origin = src["origin"].is_string() ? StringToEnum<CornerOrigin>(src["origin"].as_string(), { "topleft", "topright", "bottomleft", "bottomright" }) : CornerOrigin::TopLeft;

	if (src["textures"].is_object())
	{
		for (const auto& t : src["textures"].as_object())
		{
			textures[t.first] = new Texture(t.second.as_string());
		}
	}

	if (src["polygons"].is_array())
	{
		for (const auto& p : src["polygons"].as_array())
		{
			polygon poly;
			for (const auto& point : p.as_array())
			{
				poly.emplace_back(GetJSONVec2(point));
			}

			//ensure a closed loop
			if (poly[0] != poly[poly.size() - 1])
				poly.emplace_back(glm::vec2(poly[0]));

			polygons.emplace_back(poly);
		}
	}

	for (const auto& p : panelsSet)
	{
		auto pnl = p.as_object();
		auto panel = new Panel();

		panel->ID = GetJSONVal(pnl["id"], "");

		panel->Polygon = -1;
		panel->Shader = nullptr;

		auto const& type = pnl["type"].as_string();
		if (type == "image") panel->Type = Panel::Type::Image;
		else if (type == "text") panel->Type = Panel::Type::Text;
		else if (type == "itemicon") panel->Type = Panel::Type::ItemIcon;

		if (panel->Type == Panel::Type::Image)
		{
			panel->Texture = pnl["texture"].is_string() ? textures[pnl["texture"].as_string()] : textures.begin()->second;
			panel->Frame = GetJSONVal(pnl["frame"], 0);
			panel->Polygon = pnl["polygon"].is_integer() ? pnl["polygon"].as_integer() : -1;

			panel->Enabled = pnl["enabled"].is_boolean() ? pnl["enabled"].as_boolean() : panel->Polygon != -1;

			panel->Shader = pnl["shader"].is_string() ? Shaders[pnl["shader"].as_string()] : nullptr;
		}
		else if (panel->Type == Panel::Type::Text)
		{
			panel->Text = GetJSONVal(pnl["text"], "???");
			panel->Font = GetJSONVal(pnl["font"], 1);
			panel->Size = GetJSONVal(pnl["size"], 100.0f);

			panel->Alignment = 0;
			if (pnl["alignment"].is_string())
			{
				if (pnl["alignment"].as_string() == "right")
					panel->Alignment = 1;
				else if (pnl["alignment"].as_string() == "center")
					panel->Alignment = 2;
			}
		}
		else if (panel->Type == Panel::Type::ItemIcon)
		{
			panel->Text = GetJSONVal(pnl["text"], "");
			panel->Size = GetJSONVal(pnl["size"], 100.0f);
			panel->Polygon = pnl["polygon"].is_integer() ? pnl["polygon"].as_integer() : -1;
		}

		{
			auto pos = pnl["position"].as_array();
			auto w = 0;
			auto h = 0;
			if (panel->Type == Panel::Type::Image)
			{
				w = panel->Texture->width;
				h = panel->Texture->height;
			}
			for (int i = 0; i < 2; i++)
			{
				if (pos[i].is_string())
				{
					const auto& str = pos[i].as_string();
					if (str == "middle")
					{
						if (i == 0)
							pos[i] = (1920 * 0.5f) - (w * 0.5f);
						else
							pos[i] = (1080 * 0.5f) - (h * 0.5f);
					}
				}
			}
			panel->Position = GetJSONVec2(pnl["position"]);
		}

		panel->Angle = GetJSONVal(pnl["angle"], 0.0f);

		panel->Parent = -1;
		if (!pnl["parent"].is_null())
		{
			if (pnl["parent"].is_string())
			{
				const auto& prt = pnl["parent"].as_string();
				for (int i = 0; i < panels.size(); i++)
				{
					if (panels[i]->ID == prt)
					{ // cppcheck-suppress useStlAlgorithm
						panel->Parent = i;
						break;
					}
				}
			}
			else if(pnl["parent"].is_integer())
				panel->Parent = pnl["parent"].as_integer();
		}

		panel->Color = glm::vec4(1, 1, 1, 1);
		if (!pnl["color"].is_null())
			panel->Color = GetJSONColor(pnl["color"]);
		panel->Alpha = GetJSONVal(pnl["alpha"], 1.0f);
		panels.push_back(panel);
	}

	hasAnimations = src["anims"].is_array();
	if (hasAnimations)
	{
		for (const auto& _anim : src["anims"].as_array())
		{
			auto animObj = _anim.as_object();
			auto newAnim = Animation();
			std::string animName = animObj["name"].as_string();
			newAnim.Next = animObj["next"].as_string();
			for (const auto& _bit : animObj["bits"].as_array())
			{
				auto bitObj = _bit.as_object();
				auto newBit = AnimationBit();
				newBit.ID = bitObj["panel"].as_string();
				newBit.Property = bitObj["property"].as_string();
				newBit.FromTime = bitObj["fromTime"].as_number();
				newBit.ToTime = bitObj["toTime"].as_number();
				newBit.FromVal = bitObj["fromVal"].as_number();
				newBit.ToVal = bitObj["toVal"].as_number();

				newBit.Function = glm::linearInterpolation<float>;
				if (bitObj["easing"])
				{
					const auto& easing = bitObj["easing"].as_string();
					if (easing == "bounceOut") newBit.Function = glm::bounceEaseOut<float>;
				}

				if (newBit.FromTime >= newBit.ToTime)
				{
					conprint(2, "Animation: invalid time on {}::{} > {} to {}?", newBit.ID, newBit.Property, newBit.FromTime, newBit.ToTime);
					continue;
				}

				newAnim.Bits.push_back(newBit);
			}
			animations[animName] = newAnim;

			//temp
			if (currentAnimation.empty())
				currentAnimation = std::move(animName);
		}
	}

	animationTime = 0;
}

PanelLayout::~PanelLayout()
{
	for (auto const& t : textures)
	{
		delete t.second;
	}
}

bool PanelLayout::Tick(float dt)
{
	auto apply = [&](AnimationBit& bit, float val)
	{
		auto* panel = panels[0];
		{
			auto it = std::find_if(panels.begin(), panels.end(), [bit](auto p) { return p->ID == bit.ID; });
			if (it != panels.end())
				panel = *it;
		}
		float substitute = 0.0;

		float* prop = nullptr;
		if (bit.Property == "alpha")
			prop = &(panel->Alpha);
		else if (bit.Property == "x")
			prop = &(panel->Position.x);
		else if (bit.Property == "y")
			prop = &(panel->Position.y);
		else if (bit.Property == "frame")
			prop = &substitute;
		else if (bit.Property == "size")
			prop = &(panel->Size);

		if (prop)
		{
			*prop = val;
			if (bit.Property == "frame")
				panel->Frame = (int)val;
		}
	};

	if (hasAnimations && !currentAnimation.empty())
	{
		animationTime += dt * 1.0f;
		auto anim = animations[currentAnimation];
		auto endTime = 0.0f;
		for (const auto& bit : anim.Bits)
		{
			if (bit.ToTime > endTime)
				endTime = bit.ToTime;
		}

		if (animationTime < endTime)
		{
			for (auto& bit : anim.Bits)
			{
				if (animationTime >= bit.FromTime && animationTime <= bit.ToTime)
				{
					auto percent = (animationTime - bit.FromTime) / (bit.ToTime - bit.FromTime);
					auto val = glm::mix(bit.FromVal, bit.ToVal, bit.Function(percent));
					apply(bit, val);
				}
				else if (animationTime >= bit.ToTime)
					apply(bit, bit.ToVal);
			}
		}
		else
		{
			for (auto& bit : anim.Bits)
				apply(bit, bit.ToVal);
			currentAnimation = anim.Next;
			animationTime = 0.0;
		}

	}

	polygon poly;
	Panel* newHl = nullptr;
	//auto prevPoly = -1;
	for (const auto& panel : panels)
	{
		if (panel->Polygon == -1)
			continue;

		//if (!panel->Enabled)
		//	continue;

		auto parentPos = glm::vec2(0);
		auto parentID = panel->Parent;
		while (parentID != -1)
		{
			const auto* parent = panels[parentID];
			parentPos += parent->Position;
			parentID = parent->Parent;
		}

		//if (panel->Polygon != prevPoly)
		{
			poly.clear();
			auto const frame = panel->Texture->operator[](panel->Frame);
			auto const size = glm::vec2(frame.z, frame.w);
			for (const auto& point : polygons[panel->Polygon])
				poly.emplace_back(((point * size) + Position + parentPos + panel->Position) * scale);

			//prevPoly = panel->Polygon;
		}

		if (PointInPoly(Inputs.MousePosition, poly))
		{
			newHl = panel;
			break;
		}
	}
	if (newHl != highlighted)
		highlighted = newHl;

	if (Inputs.MouseLeft && highlighted && onClick)
	{
		Inputs.MouseLeft = false;
		if (highlighted->Enabled)
			onClick(highlighted->ID);
	}

	return true;
}

void PanelLayout::Draw(float dt)
{
	(void)(dt);
	if (!debugRenderPanelLayouts)
		return;

	for (const auto& panel : panels)
	{
		auto color = panel->Color;
		if (panel == highlighted && panel->Enabled)
			color *= 3.0f;

		auto parentPos = glm::vec2(0);
		if (Origin == CornerOrigin::TopRight || Origin == CornerOrigin::BottomRight) parentPos.x = 1920; //(float)width;
		else if (Origin == CornerOrigin::BottomLeft) parentPos.y = 1080; //(float)height;

		auto parentID = panel->Parent;
		while (parentID != -1)
		{
			const auto* parent = panels[parentID];
			parentPos += parent->Position;
			parentID = parent->Parent;
		}

		color.a = glm::clamp(Alpha * panel->Alpha, 0.0f, 1.0f);
		if (color.a == 0)
			continue;

		if (panel->Type == Panel::Type::Image)
		{
			auto texture = panel->Texture;
			auto frame = texture->operator[](panel->Frame);
			auto shader = panel->Shader ? panel->Shader : (texture->channels > 1 ? Shaders["sprite"] : Shaders["red8"]);
			auto finalPos = Position + parentPos + panel->Position;

			Sprite::DrawSprite(
				shader, *texture,
				finalPos * scale,
				glm::vec2(frame.z, frame.w) * scale,
				frame,
				panel->Angle,
				color
			);

			if (debugPanelLayoutPolygons && panel->Polygon != -1)
			{
				auto poly = polygons[panel->Polygon];
				const auto plen = poly.size();
				const auto size = glm::vec2(frame.z, frame.w);
				for (auto i = 0; i < plen; i++)
					Sprite::DrawLine((poly[i] * size) + finalPos, (poly[(i + 1) % plen] * size) + finalPos, glm::vec4(1));
			}
		}
		else if (panel->Type == Panel::Type::Text)
		{
			if (panel->Text.empty())
				continue;

			auto pos = Position + parentPos + panel->Position;
			if (panel->Alignment > 0)
			{
				auto w = Sprite::MeasureText(panel->Font, panel->Text, panel->Size * scale).x;
				if (panel->Alignment == 1)
					pos.x -= w;
				else
					pos.x -= w / 2;
			}

			Sprite::DrawText(
				panel->Font,
				panel->Text,
				pos * scale,
				color,
				panel->Size * scale,
				panel->Angle
			);
		}
		else if (panel->Type == Panel::Type::ItemIcon)
		{
			if (panel->Text.empty())
				continue;

			auto& texture = *Database::ItemIcons;
			auto frame = texture[panel->Text];
			auto shader = Shaders["sprite"];

			Sprite::DrawSprite(
				shader, texture,
				(Position + parentPos + panel->Position) * scale,
				glm::vec2(frame.z, frame.w) * (panel->Size / 100.0f) * scale,
				frame,
				panel->Angle,
				color
			);
		}
	}
}

PanelLayout::Panel* PanelLayout::GetPanel(const std::string& id)
{
	for (const auto& p : panels)
		if (p->ID == id)
			return p;
	return nullptr;
}

void PanelLayout::Play(const std::string& anim)
{
	if (!hasAnimations)
	{
		conprint(2, "Tried to play animation {} on a PanelLayout without animations.", anim);
		return;
	}
	for (const auto& a : animations)
	{
		if (a.first == anim)
		{
			currentAnimation = anim;
			animationTime = 0.0f;
			return;
		}
	}
	conprint(2, "Tried to play animation {} on a PanelLayout with no such animation.", anim);
}
