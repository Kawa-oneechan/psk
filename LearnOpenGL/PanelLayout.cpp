#include "PanelLayout.h"
#include "InputsMap.h"

PanelLayout::PanelLayout(JSONValue* source)
{
	auto src = source->AsObject();

	auto& panelsSet = src["panels"]->AsArray();

	Position = src["position"] != nullptr ? GetJSONVec2(src["position"]) : glm::vec2(0);
	Alpha = src["alpha"] != nullptr ? (float)src["alpha"]->AsNumber() : 1.0f;

	if (src["textures"] != nullptr)
	{
		for (const auto& t : src["textures"]->AsArray())
		{
			auto tfn = t->AsString();
			auto tex = new Texture(tfn);
			textures.push_back(tex);

			tfn = tfn.replace(tfn.length() - 4, 4, ".json");
			TextureAtlas atl;
			atl.push_back(glm::vec4(0, 0, tex->width, tex->height));
			GetAtlas(atl, tfn);
			atlases.push_back(atl);
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

		panel->Position = GetJSONVec2(pnl["position"]);
		auto& type = pnl["type"]->AsString();
		if (type == "image") panel->Type = 0;
		else if (type == "text") panel->Type = 1;

		if (panel->Type == 0)
		{
			panel->Texture = pnl["texture"] != nullptr ? (int)pnl["texture"]->AsNumber() : 0;
			panel->Frame = pnl["frame"] != nullptr ? (int)pnl["frame"]->AsNumber() : 0;
			panel->Polygon = pnl["polygon"] != nullptr ? (int)pnl["polygon"]->AsNumber() : -1;
		}
		else if (panel->Type == 1)
		{
			panel->Text = pnl["text"] != nullptr ? pnl["text"]->AsString() : "???";
			panel->Font = pnl["font"] != nullptr ? (int)pnl["font"]->AsNumber() : 1;
			panel->Size = pnl["size"] != nullptr ? (float)pnl["size"]->AsNumber() : 100.0f;
		}
		panel->Color = glm::vec4(1, 1, 1, 1);
		if (pnl["color"] != nullptr)
		{
			if (pnl["color"]->IsString())
			{
				auto& clr = pnl["color"]->AsString();
				if (clr == "primary")
					panel->Color = UI::primaryColor;
				else if (clr == "secondary")
					panel->Color = UI::secondaryColor;
			}
			else if (pnl["color"]->IsArray())
				panel->Color = GetJSONVec4(pnl["color"]);
		}
		panel->Alpha = pnl["alpha"] != nullptr ? (float)pnl["alpha"]->AsNumber() : 1.0f;
		panels.push_back(panel);
	}
}

void PanelLayout::Tick(double dt)
{
	if (tweens.size() > 0)
	{
		for (auto i = 0; i < tweens.size(); i++)
		{
			auto& tween = tweens[i];
			if (tween.progress() < 1.0f)
				tween.step(1); //(int)(dt * 1) + 1);
			else
				tweens.erase(tweens.begin() + i);
		}
	}

	std::vector<glm::vec2> poly;
	Panel* newHl = nullptr;
	for (const auto& panel : panels)
	{
		if (panel->Polygon == -1)
			continue;
		poly.clear();
		for (const auto& point : polygons[panel->Polygon])
			poly.emplace_back(point + Position + panel->Position);

		for (const auto& pos : poly)
			sprender->DrawSprite(whiteRect, pos, glm::vec2(6), glm::vec4(0), 0.0f, glm::vec4(1, 0, 0, 1));

		if (PointInPoly(Inputs.MousePosition, poly))
		{
			newHl = panel;
			break;
		}
	}
	if (newHl != highlighted)
		highlighted = newHl;
}

void PanelLayout::Tween(float* what, tweeny::tween<float> tween)
{
	tween.onStep([what](float v) { *what = v; return false; });
	tweens.push_back(tween);
}

void PanelLayout::Draw(double dt)
{
	for (const auto& panel : panels)
	{
		auto color = panel->Color;
		if (panel == highlighted)
			color *= 3.0f;

		color.a = clamp(Alpha * panel->Alpha, 0.0f, 1.0f);
		if (color.a == 0)
			continue;

		if (panel->Type == 0) //image
		{
			auto texture = textures[panel->Texture];
			auto frame = atlases[panel->Texture][panel->Frame];
			auto shader = spriteShader; //shaders[panel->Shader];

			sprender->DrawSprite(
				shader, texture,
				Position + panel->Position,
				glm::vec2(frame.z, frame.w),
				frame,
				0.0f,
				color,
				0
			);
		}
		else if (panel->Type == 1) //text
		{
			sprender->DrawText(
				panel->Font,
				panel->Text,
				Position + panel->Position,
				color,
				100.0f
			);
		}
	}
}

