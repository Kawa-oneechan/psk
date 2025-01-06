#include "PanelLayout.h"
#include "InputsMap.h"

bool debugPanelLayoutPolygons = false;

PanelLayout::PanelLayout(JSONValue* source)
{
	auto src = source->AsObject();

	auto& panelsSet = src["panels"]->AsArray();

	Position = src["position"] != nullptr ? GetJSONVec2(src["position"]) : glm::vec2(0);
	Alpha = src["alpha"] != nullptr ? src["alpha"]->AsNumber() : 1.0f;
	Origin = src["origin"] != nullptr ? StringToEnum<CornerOrigin>(src["origin"]->AsString(), { "topleft", "topright", "bottomleft", "bottomright" }) : CornerOrigin::TopLeft;

	if (src["textures"] != nullptr)
	{
		for (const auto& t : src["textures"]->AsArray())
		{
			textures.push_back(new Texture(t->AsString()));
		}
	}

	if (src["polygons"] != nullptr)
	{
		for (const auto& p : src["polygons"]->AsArray())
		{
			std::vector<glm::vec2> poly;
			for (const auto& point : p->AsArray())
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
		auto pnl = p->AsObject();
		auto panel = new Panel();

		if (pnl["id"] != nullptr)
		{
			panel->ID = pnl["id"]->AsString();
		}

		panel->Polygon = -1;

		auto& type = pnl["type"]->AsString();
		if (type == "image") panel->Type = Panel::Type::Image;
		else if (type == "text") panel->Type = Panel::Type::Text;
		else if (type == "itemicon") panel->Type = Panel::Type::ItemIcon;

		if (panel->Type == Panel::Type::Image)
		{
			panel->Texture = pnl["texture"] != nullptr ? pnl["texture"]->AsInteger() : 0;
			panel->Frame = pnl["frame"] != nullptr ? pnl["frame"]->AsInteger() : 0;
			panel->Polygon = pnl["polygon"] != nullptr ? pnl["polygon"]->AsInteger() : -1;

			panel->Enabled = pnl["enabled"] != nullptr ? pnl["enabled"]->AsBool() : panel->Polygon != -1;
		}
		else if (panel->Type == Panel::Type::Text)
		{
			panel->Text = pnl["text"] != nullptr ? pnl["text"]->AsString() : "???";
			panel->Font = pnl["font"] != nullptr ? pnl["font"]->AsInteger() : 1;
			panel->Size = pnl["size"] != nullptr ? pnl["size"]->AsNumber() : 100.0f;
			panel->Alignment = 0;
			if (pnl["alignment"] != nullptr)
			{
				if (pnl["alignment"]->AsString() == "right")
					panel->Alignment = 1;
				else if (pnl["alignment"]->AsString() == "center")
					panel->Alignment = 2;
			}
		}
		else if (panel->Type == Panel::Type::ItemIcon)
		{
			panel->Text = pnl["text"] != nullptr ? pnl["text"]->AsString() : "";
			panel->Size = pnl["size"] != nullptr ? pnl["size"]->AsNumber() : 100.0f;
			panel->Polygon = pnl["polygon"] != nullptr ? pnl["polygon"]->AsInteger() : -1;
		}

		{
			auto pos = pnl["position"]->AsArray();
			auto w = 0;
			auto h = 0;
			if (panel->Type == Panel::Type::Image)
			{
				w = textures[panel->Texture]->width;
				h = textures[panel->Texture]->height;
			}
			for (int i = 0; i < 2; i++)
			{
				if (pos[i]->IsString())
				{
					auto& str = pos[i]->AsString();
					if (str == "middle")
					{
						if (i == 0)
							pos[i] = new JSONValue((1920 * 0.5f) - (w * 0.5f));
						else
							pos[i] = new JSONValue((1080 * 0.5f) - (h * 0.5f));
					}
				}
			}
			panel->Position = GetJSONVec2(new JSONValue(pos));
		}

		if (pnl["angle"] != nullptr)
			panel->Angle = pnl["angle"]->AsNumber();

		panel->Parent = -1;
		if (pnl["parent"] != nullptr)
		{
			if (pnl["parent"]->IsString())
			{
				auto& prt = pnl["parent"]->AsString();
				for (int i = 0; i < panels.size(); i++)
				{
					if (panels[i]->ID == prt)
					{
						panel->Parent = i;
						break;
					}
				}
			}
			else if(pnl["parent"]->IsNumber())
				panel->Parent = pnl["parent"]->AsInteger();
		}

		panel->Color = glm::vec4(1, 1, 1, 1);
		if (pnl["color"] != nullptr)
		{
			if (pnl["color"]->IsString())
			{
				auto& clr = pnl["color"]->AsString();
				panel->Color = UI::themeColors.find(clr) != UI::themeColors.end() ? UI::themeColors[clr] : glm::vec4(1);
			}
			else if (pnl["color"]->IsArray())
				panel->Color = GetJSONColor(pnl["color"]);
		}
		panel->Alpha = pnl["alpha"] != nullptr ? pnl["alpha"]->AsNumber() : 1.0f;
		panels.push_back(panel);
	}

	hasAnimations = src["anims"] != nullptr;
	if (hasAnimations)
	{
		for (const auto& _anim : src["anims"]->AsArray())
		{
			auto animObj = _anim->AsObject();
			auto newAnim = Animation();
			std::string animName = animObj["name"]->AsString();
			newAnim.Next = animObj["next"]->AsString();
			for (const auto& _bit : animObj["bits"]->AsArray())
			{
				auto bitObj = _bit->AsObject();
				auto newBit = AnimationBit();
				newBit.ID = bitObj["panel"]->AsString();
				newBit.Property = bitObj["property"]->AsString();
				newBit.FromTime = bitObj["fromTime"]->AsNumber();
				newBit.ToTime = bitObj["toTime"]->AsNumber();
				newBit.FromVal = bitObj["fromVal"]->AsNumber();
				newBit.ToVal = bitObj["toVal"]->AsNumber();

				newBit.Function = glm::linearInterpolation<float>;
				if (bitObj["easing"])
				{
					auto& easing = bitObj["easing"]->AsString();
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

	std::vector<glm::vec2> poly;
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
	for (const auto& panel : panels)
	{
		auto color = panel->Color;
		if (panel == highlighted)
			color *= 3.0f;

		auto parentPos = glm::vec2(0);
		if (Origin == CornerOrigin::TopRight || Origin == CornerOrigin::BottomRight) parentPos.x = 1920; //(float)width;
		else if (Origin == CornerOrigin::BottomLeft || Origin == CornerOrigin::BottomRight) parentPos.y = 1080; //(float)height; //-V560

		auto parentID = panel->Parent;
		while (parentID != -1)
		{
			auto& parent = panels[parentID];
			parentPos += parent->Position;
			parentID = parent->Parent;
		}

		color.a = clamp(Alpha * panel->Alpha, 0.0f, 1.0f);
		if (color.a == 0)
			continue;

		if (panel->Type == Panel::Type::Image)
		{
			auto& texture = *textures[panel->Texture];
			auto frame = texture[panel->Frame];
			auto shader = spriteShader;
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
			auto shader = spriteShader;

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
