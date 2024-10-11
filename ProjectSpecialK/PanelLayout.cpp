#include "PanelLayout.h"
#include "InputsMap.h"

PanelLayout::PanelLayout(JSONValue* source)
{
	auto src = source->AsObject();

	auto& panelsSet = src["panels"]->AsArray();

	Position = src["position"] != nullptr ? GetJSONVec2(src["position"]) : glm::vec2(0);
	Alpha = src["alpha"] != nullptr ? src["alpha"]->AsNumber() : 1.0f;

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
		if (type == "image") panel->Type = PanelType::Image;
		else if (type == "text") panel->Type = PanelType::Text;
		else if (type == "itemicon") panel->Type = PanelType::ItemIcon;

		if (panel->Type == PanelType::Image)
		{
			panel->Texture = pnl["texture"] != nullptr ? pnl["texture"]->AsInteger() : 0;
			panel->Frame = pnl["frame"] != nullptr ? pnl["frame"]->AsInteger() : 0;
			panel->Polygon = pnl["polygon"] != nullptr ? pnl["polygon"]->AsInteger() : -1;

			panel->Enabled = pnl["enabled"] != nullptr ? pnl["enabled"]->AsBool() : panel->Polygon != -1;
		}
		else if (panel->Type == PanelType::Text)
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
		else if (panel->Type == PanelType::ItemIcon)
		{
			panel->Text = pnl["text"] != nullptr ? pnl["text"]->AsString() : "orestone";
			panel->Size = pnl["size"] != nullptr ? pnl["size"]->AsNumber() : 100.0f;
			panel->Polygon = pnl["polygon"] != nullptr ? pnl["polygon"]->AsInteger() : -1;
		}

		{
			auto pos = pnl["position"]->AsArray();
			auto w = 0;
			auto h = 0;
			if (panel->Type == PanelType::Image)
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
							pos[i] = new JSONValue((width * 0.5f) - (w * 0.5f));
						else
							pos[i] = new JSONValue((height * 0.5f) - (h * 0.5f));
					}
				}
			}
			panel->Position = GetJSONVec2(new JSONValue(pos));
		}

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
}

void PanelLayout::Tick(float dt)
{
	dt;
	if (tweens.size() > 0)
	{
		for (auto i = 0; i < tweens.size(); i++)
		{
			auto& tween = tweens[i];
			if (tween.step())
				tweens.erase(tweens.begin() + i);
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

		if (panel->Type == PanelType::Image)
		{
			auto& texture = *textures[panel->Texture];
			auto frame = texture[panel->Frame];
			auto shader = spriteShader;

			sprender.DrawSprite(
				shader, texture,
				(Position + parentPos + panel->Position) * scale,
				glm::vec2(frame.z, frame.w) * scale,
				frame,
				0.0f,
				color
			);
		}
		else if (panel->Type == PanelType::Text)
		{
			if (panel->Text.empty())
				continue;

			auto pos = Position + parentPos + panel->Position;
			if (panel->Alignment > 0)
			{
				auto w = sprender.MeasureText(panel->Font, panel->Text, panel->Size * scale).x;
				if (panel->Alignment == 1)
					pos.x -= w;
				else
					pos.x -= w / 2;
			}

			sprender.DrawText(
				panel->Font,
				panel->Text,
				pos * scale,
				color,
				panel->Size * scale
			);
		}
		else if (panel->Type == PanelType::ItemIcon)
		{
			if (panel->Text.empty())
				continue;

			auto& texture = *Database::ItemIcons;
			auto frame = Database::ItemIconAtlas[panel->Text];
			auto shader = spriteShader;

			sprender.DrawSprite(
				shader, texture,
				(Position + parentPos + panel->Position) * scale,
				glm::vec2(frame.z, frame.w) * (panel->Size / 100.0f) * scale,
				frame,
				0.0f,
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
