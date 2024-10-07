#include "SpecialK.h"
#include "Town.h"

#include "support/ImGUI/imgui.h"
//#include "support/ImGUI/imgui_stdlib.h"
#include "support/ImGUI/imgui_impl_glfw.h"
#include "support/ImGUI/imgui_impl_opengl3.h"

bool debuggerEnabled{ true };

extern float uiTime, glTime;
extern GLFWwindow* window;

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

	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Target");
		{
			auto& tar = MainCamera.GetTarget();
			if (ImGui::DragFloat("X", &tar.x, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Y", &tar.y, 1.0, -50, 50))
				MainCamera.Update();
			if (ImGui::DragFloat("Z", &tar.z, 1.0, -100, 100))
				MainCamera.Update();
		}

		if (ImGui::DragFloat("Distance", &MainCamera.GetDistance(), 1.0, -100, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
			MainCamera.Update();

		ImGui::Text("Angles");
		{
			auto& ang = MainCamera.GetAngles();
			if (ImGui::DragFloat("Roll", &ang.x, 1.0, -359, 359))
				MainCamera.Update();
			if (ImGui::DragFloat("Pitch", &ang.y, 1.0, -359, 359))
				MainCamera.Update();
			if (ImGui::DragFloat("Yaw", &ang.z, 1.0, -359, 359))
				MainCamera.Update();
		}

		auto dummy = true;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Drum", &dummy);
		ImGui::Checkbox("Locked", &dummy);
		ImGui::EndDisabled();
			
		if (ImGui::Button("Reset"))
		{
			MainCamera.Target(glm::vec3(0, 6, 0));
			MainCamera.Angles(glm::vec3(0, 20, 0));
			MainCamera.Distance(60);
			MainCamera.Update();
		}

		if (ImGui::Button("Copy JSON"))
		{
			std::string json;
			json += "{\n";
			auto tar = MainCamera.GetTarget();
			auto ang = MainCamera.GetAngles();
			auto dis = MainCamera.GetDistance();
			json += fmt::format("\t\"target\": [{}, {}, {}],\n", tar[0], tar[1], tar[2]);
			json += fmt::format("\t\"angles\": [{}, {}, {}],\n", ang[0], ang[1], ang[2]);
			json += fmt::format("\t\"distance\": {}\n", dis);
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

	static VillagerP debugVillager = town.Villagers[0];
	static VillagerP debugAllVillager = villagers[0];

	if (ImGui::Begin("Villagers"))
	{
		if (ImGui::BeginTabBar("Villagers"))
		{
			if (ImGui::BeginTabItem("In town"))
			{
				if (ImGui::BeginListBox("##townVillagers", ImVec2(-FLT_MIN, (town.Villagers.size() + 1) * ImGui::GetTextLineHeightWithSpacing())))
				{
					auto amount = town.Villagers.size();
					for (int i = 0; i < amount; i++)
					{
						const bool selected = (town.Villagers[i] == debugVillager);
						if (ImGui::Selectable(town.Villagers[i]->Name().c_str(), selected))
						{
							debugVillager = town.Villagers[i];
							debugAllVillager = debugVillager;
						}
					}
					ImGui::EndListBox();
				}

				if (debugVillager->Icon)
					ImGui::Image((void*)(uintptr_t)debugVillager->Icon->ID, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

				ImGui::Text(debugVillager->ID.c_str());

				ImGui::SliderInt("Face", &debugVillager->face, 0, 15);
				ImGui::SliderInt("Mouth", &debugVillager->mouth, 0, 8);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("All"))
			{
				if (ImGui::BeginListBox("##allVillagers", ImVec2(-FLT_MIN, 12 * ImGui::GetTextLineHeightWithSpacing())))
				{
					auto amount = villagers.size();
					for (int i = 0; i < amount; i++)
					{
						const bool selected = (villagers[i] == debugAllVillager);
						if (ImGui::Selectable(villagers[i]->Name().c_str(), selected))
						{
							debugAllVillager = villagers[i];
						}
					}
					ImGui::EndListBox();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	if (ImGui::Begin("Player"))
	{
		ImGui::InputInt("Bells", (int*)&thePlayer.Bells, 10, 100);
	}
	ImGui::End();

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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
