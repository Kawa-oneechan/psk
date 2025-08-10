#include "PanelLayout.h"
#include "engine/InputsMap.h"
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Console.h"
#include "engine/SpriteRenderer.h"
#include "Database.h"
#include "Types.h"

bool debugPanelLayoutPolygons = false;
bool debugRenderPanelLayouts = true;

PanelLayout::PanelLayout(jsonValue& source)
{
	auto src = source.as_object();

	auto& panelsSet = src["panels"].as_array();

	Position = src["position"].is_array() ? GetJSONVec2(src["position"]) : glm::vec2(0);
	Alpha = src["alpha"].is_number() ? src["alpha"].as_number() : 1.0f;
	Origin = src["origin"].is_string() ? StringToEnum<CornerOrigin>(src["origin"].as_string(), { "topleft", "topright", "bottomleft", "bottomright" }) : CornerOrigin::TopLeft;

	if (src["textures"].is_array())
	{
		for (const auto& t : src["textures"].as_array())
		{
			textures.push_back(new Texture(t.as_string()));
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

		if (pnl["id"].is_string())
		{
			panel->ID = pnl["id"].as_string();
		}

		panel->Polygon = -1;

		auto& type = pnl["type"].as_string();
		if (type == "image") panel->Type = Panel::Type::Image;
		else if (type == "text") panel->Type = Panel::Type::Text;
		else if (type == "itemicon") panel->Type = Panel::Type::ItemIcon;

		if (panel->Type == Panel::Type::Image)
		{
			panel->Texture = pnl["texture"].is_integer() ? pnl["texture"].as_integer() : 0;
			panel->Frame = pnl["frame"].is_integer() ? pnl["frame"].as_integer() : 0;
			panel->Polygon = pnl["polygon"].is_integer() ? pnl["polygon"].as_integer() : -1;

			panel->Enabled = pnl["enabled"].is_boolean() ? pnl["enabled"].as_boolean() : panel->Polygon != -1;
		}
		else if (panel->Type == Panel::Type::Text)
		{
			panel->Text = pnl["text"].is_string() ? pnl["text"].as_string() : "???";
			panel->Font = pnl["font"].is_integer() ? pnl["font"].as_integer() : 1;
			panel->Size = pnl["size"].is_number() ? pnl["size"].as_number() : 100.0f;
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
			panel->Text = pnl["text"].is_string() ? pnl["text"].as_string() : "";
			panel->Size = pnl["size"].is_number() ? pnl["size"].as_number() : 100.0f;
			panel->Polygon = pnl["polygon"].is_integer() ? pnl["polygon"].as_integer() : -1;
		}

		{
			auto pos = pnl["position"].as_array();
			auto w = 0;
			auto h = 0;
			if (panel->Type == Panel::Type::Image)
			{
				w = textures[panel->Texture]->width;
				h = textures[panel->Texture]->height;
			}
			for (int i = 0; i < 2; i++)
			{
				if (pos[i].is_string())
				{
					auto& str = pos[i].as_string();
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

		if (pnl["angle"].is_number())
			panel->Angle = pnl["angle"].as_number();

		panel->Parent = -1;
		if (!pnl["parent"].is_null())
		{
			if (pnl["parent"].is_string())
			{
				auto& prt = pnl["parent"].as_string();
				for (int i = 0; i < panels.size(); i++)
				{
					if (panels[i]->ID == prt)
					{
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
		{
			if (pnl["color"].is_string())
			{
				auto& clr = pnl["color"].as_string();
				panel->Color = UI::themeColors.find(clr) != UI::themeColors.end() ? UI::themeColors[clr] : glm::vec4(1);
			}
			else if (pnl["color"].is_array())
				panel->Color = GetJSONColor(pnl["color"]);
		}
		panel->Alpha = pnl["alpha"].is_number() ? pnl["alpha"].as_number() : 1.0f;
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
					auto& easing = bitObj["easing"].as_string();
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

bool PanelLayout::Tick(float dt)
{
	if (tweens.size() > 0)
	{
		for (auto i = 0; i < tweens.size(); i++)
		{
			auto& tween = tweens[i];
			if (tween.step())
				tweens.erase(tweens.begin() + i);
		}
	}

	if (hasAnimations && !currentAnimation.empty())
	{
		animationTime += dt * 1.0f;
		auto anim = animations[currentAnimation];
		auto endTime = 0.0f;
		for (auto& bit : anim.Bits)
		{
			if (bit.ToTime > endTime)
				endTime = bit.ToTime;
		}
		if (animationTime >= endTime)
		{
			currentAnimation = anim.Next;
			animationTime = 0.0;
		}
		else
		{
			for (auto& bit : anim.Bits)
			{
				auto* panel = panels[0];
				for (const auto& p : panels)
				{
					if (p->ID == bit.ID)
					{
						panel = p;
						break;
					}
				}
				float subsitute = 0.0;

				float* prop = nullptr;
				if (bit.Property == "alpha")
					prop = &(panel->Alpha);
				else if (bit.Property == "x")
					prop = &(panel->Position.x);
				else if (bit.Property == "y")
					prop = &(panel->Position.y);
				else if (bit.Property == "frame")
					prop = &subsitute;
				else if (bit.Property == "size")
					prop = &(panel->Size);

				if (prop == nullptr)
					continue;

				/*
				if (animationTime < bit.FromTime)
				{
					*prop = bit.FromVal;
				}
				else if (animationTime > bit.ToTime)
				{
				}
				else
				*/
				if (animationTime >= bit.FromTime && animationTime <= bit.ToTime)
				{
					//auto duration = bit.ToTime - bit.FromTime;
					auto percent = (animationTime - bit.FromTime) / (bit.ToTime - bit.FromTime);
					*prop = glm::mix(bit.FromVal, bit.ToVal, bit.Function(percent));

					if (bit.Property == "frame")
						panel->Frame = (int)glm::floor(subsitute);
				}
			}
		}
	}

	polygon poly;
	Panel* newHl = nullptr;
	//auto prevPoly = -1;
	for (const auto& panel : panels)
	{
		if (panel->Polygon == -1)
			continue;

		if (!panel->Enabled)
			continue;

		auto parentPos = glm::vec2(0);
		auto parentID = panel->Parent;
		while (parentID != -1)
		{
			auto& parent = panels[parentID];
			parentPos += parent->Position;
			parentID = parent->Parent;
		}

		//if (panel->Polygon != prevPoly)
		{
			poly.clear();
			auto const frame = textures[panel->Texture]->operator[](panel->Frame);
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

	return true;
}

Tween<float>* PanelLayout::Tween(float* target, float from, float to, float speed, std::function<float(float)> interpolator)
{
	auto tween = new ::Tween<float>(target, from, to, speed, interpolator);
	tweens.push_back(*tween);
	return tween;
}

void PanelLayout::Draw(float dt)
{
	dt;
	if (!debugRenderPanelLayouts)
		return;

	for (const auto& panel : panels)
	{
		auto color = panel->Color;
		if (panel == highlighted)
			color *= 3.0f;

		auto parentPos = glm::vec2(0);
		if (Origin == CornerOrigin::TopRight || Origin == CornerOrigin::BottomRight) parentPos.x = 1920; //(float)width;
		else if (Origin == CornerOrigin::BottomLeft) parentPos.y = 1080; //(float)height;

		auto parentID = panel->Parent;
		while (parentID != -1)
		{
			auto& parent = panels[parentID];
			parentPos += parent->Position;
			parentID = parent->Parent;
		}

		color.a = glm::clamp(Alpha * panel->Alpha, 0.0f, 1.0f);
		if (color.a == 0)
			continue;

		if (panel->Type == Panel::Type::Image)
		{
			auto& texture = *textures[panel->Texture];
			auto frame = texture[panel->Frame];
			auto shader = Shaders["sprite"];
			auto finalPos = Position + parentPos + panel->Position;

			Sprite::DrawSprite(
				shader, texture,
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
			auto frame = Database::ItemIconAtlas[panel->Text];
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
	for (auto& a : animations)
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
