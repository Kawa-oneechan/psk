#include "SpecialK.h"
#include "Town.h"

#include "support/ImGUI/imgui.h"
#include "support/ImGUI/imgui_impl_glfw.h"
#include "support/ImGUI/imgui_impl_opengl3.h"

bool debuggerEnabled{ true };

extern float uiTime, glTime;
extern GLFWwindow* window;

extern glm::vec4 lightPos[MaxLights];
extern glm::vec4 lightCol[MaxLights];

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
		if (ImGui::BeginTabBar("Camera"))
		{
			if (ImGui::BeginTabItem("Position"))
			{
				if (ImGui::SliderFloat("X", &MainCamera.Position.x, -50, 50))
					MainCamera.UpdateCameraVectors();
				if (ImGui::SliderFloat("Y", &MainCamera.Position.y, -50, 50))
					MainCamera.UpdateCameraVectors();
				if (ImGui::SliderFloat("Z", &MainCamera.Position.z, -100, 100))
					MainCamera.UpdateCameraVectors();

				if (ImGui::SliderFloat("Pitch", &MainCamera.Pitch, -89, 89))
					MainCamera.UpdateCameraVectors();
				if (ImGui::SliderFloat("Yaw", &MainCamera.Yaw, -50, 50))
					MainCamera.UpdateCameraVectors();

				if (ImGui::Button("Reset"))
				{
					MainCamera.Position.x = 0;
					MainCamera.Position.y = 5;
					MainCamera.Position.z = 50;
					MainCamera.Pitch = 0;
					MainCamera.Yaw = 0;
					MainCamera.UpdateCameraVectors();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Target"))
			{
				ImGui::Checkbox("Free cam", &MainCamera.Free);

				ImGui::SliderFloat("X", &MainCamera.Target.x, -50, 50);
				ImGui::SliderFloat("Y", &MainCamera.Target.y, -50, 50);
				ImGui::SliderFloat("Z", &MainCamera.Target.z, -100, 100);

				if (ImGui::Button("Reset"))
				{
					MainCamera.Target.x = 0;
					MainCamera.Target.y = 0;
					MainCamera.Target.z = 0;
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
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
			ImGui::SliderFloat("X", &lightPos[lightIndex].x, -50, 50);
			ImGui::SliderFloat("Y", &lightPos[lightIndex].y, -50, 50);
			ImGui::SliderFloat("Z", &lightPos[lightIndex].z, -50, 50);

			if (ImGui::Button("Reset"))
			{
				lightPos[lightIndex].x = 0;
				lightPos[lightIndex].y = 15;
				lightPos[lightIndex].z = 20;
			}
		}

		ImGui::ColorPicker4("Color", &lightCol[lightIndex].x);

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
			auto json = JSON::Parse(ImGui::GetClipboardText());
			auto i = 0;
			for (auto& lobj : json->AsArray())
			{
				auto l = lobj->AsObject();
				lightPos[i] = GetJSONVec4(l["pos"]);
				lightCol[i] = GetJSONColor(l["col"]);
				i++;
				if (i == MaxLights)
					break;
			}
		}

	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
