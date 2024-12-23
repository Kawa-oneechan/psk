#include "SpecialK.h"
#include "Town.h"

#include "support/ImGUI/imgui.h"
#include "support/ImGUI/imgui_impl_glfw.h"
#include "support/ImGUI/imgui_impl_opengl3.h"

bool debuggerEnabled{ false };

extern float uiTime, glTime;
extern GLFWwindow* window;
extern float boneTester;

bool IsImGuiHovered()
{
	return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered();
}

void SetupImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

static void DoCamera()
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::SeparatorText("Target");
		{
			auto& tar = MainCamera.GetTarget();
			if (ImGui::DragFloat("X", &tar.x, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Y", &tar.y, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Z", &tar.z, 1.0, -100, 100))
				MainCamera.Update();
		}

		ImGui::SeparatorText("Angles");
		{
			auto& ang = MainCamera.GetAngles();
			if (ImGui::DragFloat("Roll", &ang.x, 1.0, -359, 359))
				MainCamera.Update();
			if (ImGui::DragFloat("Pitch", &ang.y, 1.0, -359, 359))
				MainCamera.Update();
			if (ImGui::DragFloat("Yaw", &ang.z, 1.0, -359, 359))
				MainCamera.Update();
		}

		ImGui::SeparatorText("Offset");
		{
			auto& off = MainCamera.GetOffset();
			if (ImGui::DragFloat("X##ox", &off.x, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Y##oy", &off.y, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Z##oz", &off.z, 1.0, -100, 100))
				MainCamera.Update();
		}
		

		ImGui::Separator();
		if (ImGui::DragFloat("Distance", &MainCamera.GetDistance(), 1.0, -100, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
			MainCamera.Update();


		ImGui::SeparatorText("Settings");
		ImGui::Checkbox("Drum", &MainCamera.Drum);
		ImGui::DragFloat("Drum amount", &MainCamera.DrumAmount, 0.001f, -1.0, 1.0);
		ImGui::DragFloat("Drum power", &MainCamera.DrumPower, 0.25, -2.0, 2.0);
		ImGui::Checkbox("Locked", &MainCamera.Locked);

		if (ImGui::Button("Reset"))
		{
			MainCamera.Target(glm::vec3(0, 0, 0));
			MainCamera.Angles(glm::vec3(0, 41, 0));
			MainCamera.Offset(glm::vec3(0, -3, 0));
			MainCamera.Distance(70);
			MainCamera.Update();
		}

		if (ImGui::Button("Copy JSON"))
		{
			std::string json;
			json += "{\n";
			auto tar = MainCamera.GetTarget();
			auto ang = MainCamera.GetAngles();
			auto off = MainCamera.GetOffset();
			auto dis = MainCamera.GetDistance();
			json += fmt::format("\t\"target\": [{}, {}, {}],\n", tar[0], tar[1], tar[2]);
			json += fmt::format("\t\"angles\": [{}, {}, {}],\n", ang[0], ang[1], ang[2]);
			json += fmt::format("\t\"offset\": [{}, {}, {}],\n", off[0], off[1], off[2]);
			json += fmt::format("\t\"distance\": {},\n", dis);
			json += fmt::format("\t\"drum\": {},\n", MainCamera.Drum);
			json += fmt::format("\t\"drumAmount\": {},\n", MainCamera.DrumAmount);
			json += fmt::format("\t\"drumPower\": {}\n", MainCamera.DrumPower);
			json += "}\n";
			ImGui::SetClipboardText(json.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Paste JSON"))
		{
			std::string result = "";
			try
			{
				auto json = JSON::Parse(ImGui::GetClipboardText());
				LoadCamera(json);
			}
			catch (std::runtime_error& x)
			{
				result = x.what();
			}
		}
	}
	ImGui::End();
}

static void DoLights()
{
	static int lightIndex = 0;
	static const char lightLabels[] = "1\0" "2\0" "3\0" "4\0" "5\0" "6\0" "7\0" "8\0";
	if (ImGui::Begin("Lighting"))
	{
		for (int i = 0; i < MaxLights; i++)
		{
			bool selected = lightIndex == i;
			if (ImGui::RadioButton(lightLabels + (i * 2), selected))
			{
				lightIndex = i;
			}
			if (i < 3)
				ImGui::SameLine();
		}

		ImGui::Text("Position");
		{
			ImGui::DragFloat("X", &lightPos[lightIndex].x, 1.0, -50, 50);
			ImGui::DragFloat("Y", &lightPos[lightIndex].y, 1.0, -50, 50);
			ImGui::DragFloat("Z", &lightPos[lightIndex].z, 1.0, -50, 50);

			if (ImGui::Button("Reset"))
			{
				if (lightIndex == 0)
				{
					lightPos[lightIndex] = { 0.0f, 15.0f, 20.0f, 0 };
					lightCol[lightIndex] = { 1.0f, 1.0f, 1.0f, 0.25f };
				}
				else
				{
					lightPos[lightIndex] = { 0, 0, 0, 0 };
					lightCol[lightIndex] = { 0, 0, 0, 0 };
				}
			}
		}

		ImGui::ColorPicker4("Color", &lightCol[lightIndex].x, ImGuiColorEditFlags_DisplayRGB);

		if (ImGui::Button("Copy JSON"))
		{
			std::string json;
			std::vector<std::string> blocks;
			json += "[\n";
			for (int i = 0; i < MaxLights; i++)
			{
				if (lightCol[i][3] == 0)
					continue;
				std::string block;
				block += "\t{\n";
				block += fmt::format("\t\t\"pos\": [{}, {}, {}, {}],\n", lightPos[i][0], lightPos[i][1], lightPos[i][2], lightPos[i][3]);
				block += fmt::format("\t\t\"col\": [{}, {}, {}, {}] \n", lightCol[i][0], lightCol[i][1], lightCol[i][2], lightCol[i][3]);
				block += "\t}";
				blocks.push_back(block);
			}
			json += join(blocks.begin(), blocks.end(), ",\n", "\n");
			json += "]\n";
			ImGui::SetClipboardText(json.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Paste JSON"))
		{
			std::string result = "";
			try
			{
				auto json = JSON::Parse(ImGui::GetClipboardText());
				LoadLights(json);
			}
			catch (std::runtime_error& x)
			{
				result = x.what();
			}
		}

	}
	ImGui::End();
}

static void DoVillager()
{
	//TODO: use *current* map.
	static VillagerP debugVillager = villagers[0];
	if (ImGui::Begin("Villagers"))
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
		{
			if (ImGui::BeginListBox("##villagers", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
			{
				auto amount = villagers.size();
				for (int i = 0; i < amount; i++)
				{
					if (villagers[i]->IsSpecial())
						continue;

					const bool selected = (villagers[i] == debugVillager);
					const bool here = std::find(town.Villagers.begin(), town.Villagers.end(), villagers[i]) != std::end(town.Villagers);

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, here ? 0.0f : 1.0f, villagers[i]->IsManifest() ? 1.0f : 0.5f));
					if (ImGui::Selectable(villagers[i]->Name().c_str(), selected))
					{
						debugVillager = villagers[i];
					}
					ImGui::PopStyleColor(1);
				}
				ImGui::EndListBox();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
		{
			ImGui::Text(debugVillager->Name().c_str());
			ImGui::Separator();

			const bool here = std::find(town.Villagers.begin(), town.Villagers.end(), debugVillager) != std::end(town.Villagers);

			if (debugVillager->Icon)
			{
				debugVillager->Icon->Use();
				ImGui::Image(debugVillager->Icon->ID, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
			}

			if (!debugVillager->IsManifest() && ImGui::Button("Manifest"))
				debugVillager->Manifest();
			else if (debugVillager->IsManifest() && ImGui::Button("Depart"))
				debugVillager->Depart();
			ImGui::SameLine();
			if (!here && ImGui::Button("Bring in"))
				town.Villagers.push_back(debugVillager);
			else if (here && ImGui::Button("Remove"))
			{
				auto there = std::find(town.Villagers.begin(), town.Villagers.end(), debugVillager);
				if (there != town.Villagers.end())
					town.Villagers.erase(there);
			}

			ImGui::Text(debugVillager->ID.c_str());

			if (!debugVillager->IsManifest())
				ImGui::BeginDisabled();

			if (ImGui::Button("Reload textures"))
				debugVillager->ReloadTextures();

			ImGui::SeparatorText("Animation");
			ImGui::SliderInt("Face", &debugVillager->face, 0, 15);
			ImGui::SliderInt("Mouth", &debugVillager->mouth, 0, 8);

			ImGui::SeparatorText("Position");
			auto& tar = debugVillager->Position;
			ImGui::DragFloat("X", &tar.x, 1.0, -50, 50);
			ImGui::DragFloat("Y", &tar.y, 1.0, -50, 50);
			ImGui::DragFloat("Z", &tar.z, 1.0, -100, 100);
			ImGui::DragFloat("Facing", &debugVillager->Facing, 1.0, -100, 100);

			if (!debugVillager->IsManifest())
				ImGui::EndDisabled();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

static void DoPlayer()
{
	if (ImGui::Begin("Player"))
	{
		ImGui::InputInt("Bells", (int*)&thePlayer.Bells, 10, 100);
		ImGui::DragFloat("BONE TESTER", &boneTester, 1.0, -359, 359);
	}
	ImGui::End();
}

void DoImGui()
{
	if (!debuggerEnabled)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Timing"))
	{
		ImGui::Text("UI: %f\nGL: %f", uiTime, glTime);
	}
	ImGui::End();

	DoCamera();
	DoLights();
	DoVillager();
	DoPlayer();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
