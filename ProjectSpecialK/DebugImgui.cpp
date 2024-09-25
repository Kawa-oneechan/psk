#include "SpecialK.h"
#include "Town.h"

#include "support/ImGUI/imgui.h"
#include "support/ImGUI/imgui_impl_glfw.h"
#include "support/ImGUI/imgui_impl_opengl3.h"

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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Timing"))
	{
		ImGui::Text("UI: %f\nGL: %f", uiTime, glTime);
		ImGui::End();
	}

	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position: %f %f %f", MainCamera.Position.x, MainCamera.Position.y, MainCamera.Position.z);
		ImGui::Text("Pitch/Yaw: %f %f", MainCamera.Pitch, MainCamera.Yaw);
		ImGui::Text("Target: %f %f %f", MainCamera.Target.x, MainCamera.Target.y, MainCamera.Target.z);
		ImGui::Checkbox("Free", &MainCamera.Free);
		ImGui::End();
	}

	//lightPos.x, lightPos.y, lightPos.z

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
		ImGui::End();
	}

	if (ImGui::Begin("Player"))
	{
		ImGui::InputInt("Bells", (int*)&thePlayer.Bells, 10, 100);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
