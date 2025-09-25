#include <ImGUI/imgui.h>
#include "engine/TextUtils.h"
#include "engine/Utilities.h"
#include "engine/Console.h"
#include "engine/Game.h"
#include "Game.h"
#include "Town.h"
#include "Animator.h"
#include "Utilities.h"
#include "Camera.h"
#include "Database.h"
#include "Player.h"

static void DoCamera()
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::SeparatorText("Target");
		{
			auto& tar = MainCamera->GetTarget();
			if (ImGui::DragFloat("X", &tar.x, 1.0, -50, 50))
				MainCamera->Update();
			if (ImGui::DragFloat("Y", &tar.y, 1.0, -50, 50))
				MainCamera->Update();
			if (ImGui::DragFloat("Z", &tar.z, 1.0, -100, 100))
				MainCamera->Update();
		}

		ImGui::SeparatorText("Angles");
		{
			auto& ang = MainCamera->GetAngles();
			if (ImGui::DragFloat("Roll", &ang.x, 1.0, -359, 359))
				MainCamera->Update();
			if (ImGui::DragFloat("Pitch", &ang.y, 1.0, -359, 359))
				MainCamera->Update();
			if (ImGui::DragFloat("Yaw", &ang.z, 1.0, -359, 359))
				MainCamera->Update();
		}

		ImGui::Separator();
		if (ImGui::DragFloat("Distance", &MainCamera->GetDistance(), 1.0, -100, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
			MainCamera->Update();


		ImGui::SeparatorText("Settings");
		ImGui::Checkbox("Drum", &commonUniforms.CurveEnabled);
		ImGui::DragFloat("Drum amount", &commonUniforms.CurveAmount, 0.001f, -1.0, 1.0);
		ImGui::DragFloat("Drum power", &commonUniforms.CurvePower, 0.25, -2.0, 2.0);
		ImGui::Checkbox("Locked", &MainCamera->Locked);

		if (ImGui::Button("Reset"))
		{
			MainCamera->Target(glm::vec3(0, 0, 0));
			MainCamera->Angles(glm::vec3(0, 46, 0));
			MainCamera->Distance(70);
			MainCamera->Update();
		}

		if (ImGui::Button("Copy JSON"))
		{
			std::string json;
			json += "{\n";
			auto tar = MainCamera->GetTarget();
			auto ang = MainCamera->GetAngles();
			auto dis = MainCamera->GetDistance();
			json += fmt::format("\t\"target\": [{}, {}, {}],\n", tar[0], tar[1], tar[2]);
			json += fmt::format("\t\"angles\": [{}, {}, {}],\n", ang[0], ang[1], ang[2]);
			json += fmt::format("\t\"distance\": {},\n", dis);
			json += fmt::format("\t\"drum\": {},\n", commonUniforms.CurveEnabled == 1);
			json += fmt::format("\t\"drumAmount\": {},\n", commonUniforms.CurveAmount);
			json += fmt::format("\t\"drumPower\": {}\n", commonUniforms.CurvePower);
			json += "}\n";
			ImGui::SetClipboardText(json.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Paste JSON"))
		{
			try
			{
				auto json = json5pp::parse5(ImGui::GetClipboardText());
				LoadCamera(json);
			}
			catch (std::runtime_error& x)
			{
				conprint(4, x.what());
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
			if (i % (MaxLights / 2) != (MaxLights / 2) - 1)
				ImGui::SameLine();
		}

		auto& l = commonUniforms.Lights[lightIndex];

		ImGui::Text("Position");
		{
			ImGui::DragFloat("X", &l.pos.x, 1.0, -50, 50);
			ImGui::DragFloat("Y", &l.pos.y, 1.0, -50, 50);
			ImGui::DragFloat("Z", &l.pos.z, 1.0, -50, 50);

			if (ImGui::Button("Reset"))
			{
				if (lightIndex == 0)
				{
					l.pos = { 0.0f, 15.0f, 20.0f, 0 };
					l.color = { 1.0f, 1.0f, 1.0f, 0.25f };
				}
				else
				{
					l.pos = { 0, 0, 0, 0 };
					l.color = { 0, 0, 0, 0 };
				}
			}
		}

		ImGui::ColorPicker4("Color", &l.color.x, ImGuiColorEditFlags_DisplayRGB);

		if (ImGui::Button("Copy JSON"))
		{
			std::string json;
			std::vector<std::string> blocks;
			json += "[\n";
			for (int i = 0; i < MaxLights; i++)
			{
				l = commonUniforms.Lights[i];
				if (l.color[3] == 0)
					continue;
				std::string block;
				block += "\t{\n";
				block += fmt::format("\t\t\"pos\": [{}, {}, {}, {}],\n", l.pos[0], l.pos[1], l.pos[2], l.pos[3]);
				block += fmt::format("\t\t\"col\": [{}, {}, {}, {}] \n", l.color[0], l.color[1], l.color[2], l.color[3]);
				block += "\t}";
				blocks.push_back(block);
			}
			json += StringJoin(blocks.begin(), blocks.end(), ",\n", "\n");
			json += "]\n";
			ImGui::SetClipboardText(json.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Paste JSON"))
		{
			try
			{
				auto json = json5pp::parse5(ImGui::GetClipboardText());
				LoadLights(json);
			}
			catch (std::runtime_error& x)
			{
				conprint(4, x.what());
			}
		}

	}
	ImGui::End();
}

static Armature* debugArmature = nullptr;
static int selectedJoint = 0;

static void DoVillager()
{
	//TODO: use *current* map.
	static VillagerP debugVillager = villagers[0];
	auto& townVillagers = town->Villagers;
	
	if (ImGui::Begin("Villagers"))
	{
		ImGui::BeginChild("left pane", ImVec2(170, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
		{
			auto amount = villagers.size();
			std::string lastSpecies = "__special__";
			ImGui::SeparatorText("special");
			for (int i = 0; i < amount; i++)
			{
				if (villagers[i]->Species()->ID != lastSpecies)
				{
					lastSpecies = villagers[i]->Species()->ID;
					auto newSpecies = StripBJTS(villagers[i]->SpeciesName());
					ImGui::SeparatorText(newSpecies.c_str());
				}

				const bool selected = (villagers[i] == debugVillager);
				const bool here = std::find(townVillagers.begin(), townVillagers.end(), villagers[i]) != std::end(townVillagers);

				ImGui::PushStyleColor(ImGuiCol_Text, here ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : villagers[i]->IsManifest() ? ImVec4(0.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				if (ImGui::Selectable(villagers[i]->Name().c_str(), selected))
				{
					debugVillager = villagers[i];
				}
				ImGui::PopStyleColor(1);
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
		{
			ImGui::Text(debugVillager->Name().c_str());
			ImGui::Separator();

			const bool here = std::find(townVillagers.begin(), townVillagers.end(), debugVillager) != std::end(townVillagers);

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
				town->Villagers.push_back(debugVillager);
			else if (here && ImGui::Button("Remove"))
			{
				townVillagers = town->Villagers;
				auto there = std::find(townVillagers.begin(), townVillagers.end(), debugVillager);
				if (there != townVillagers.end())
					townVillagers.erase(there);
			}

			if (here)
			{
				ImGui::SameLine();
				if (ImGui::Button("Armature"))
				{
					//debugArmature = &debugVillager->Model()->Bones;
					debugArmature = &debugVillager->Animator()->Bones;
				}
			}

			ImGui::Text(debugVillager->ID.c_str());

			if (!debugVillager->IsManifest())
				ImGui::BeginDisabled();

			if (ImGui::Button("Reload textures"))
				debugVillager->ReloadTextures();
			ImGui::SameLine();
			if (ImGui::Button("Track"))
				MainCamera->Target(&debugVillager->Position);

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
		if (ImGui::Button("Armature"))
		{
			debugArmature = &thePlayer.Model()->Bones;
		}
		ImGui::SameLine();
		if (ImGui::Button("Track"))
		{
			MainCamera->Target(&thePlayer.Position);
		}

		ImGui::SeparatorText("Animation");
		ImGui::SliderInt("Face", &thePlayer.face, 0, 1);
		ImGui::SliderInt("Mouth", &thePlayer.mouth, 0, 1);
	}
	ImGui::End();
}

static void traverseArmature(int origin)
{
	Model::Bone& thisJoint = debugArmature->at(origin);
	int flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (thisJoint.Children.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (selectedJoint == origin)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(thisJoint.Name.c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selectedJoint = origin;
		}
		for (auto i : thisJoint.Children)
		{
			traverseArmature(i);
		}
		ImGui::TreePop();
	}
}

static void applyPose(jsonValue& json)
{
	auto j = json.as_object();
	//reset first
	for (auto& bone : *debugArmature)
		bone.Rotation = glm::vec3(0.0f);
	for (auto& b : j)
	{
		auto& name = b.first;
		auto trns = b.second.as_object();
		for (auto& bone : *debugArmature)
		{
			if (bone.Name == name)
			{
				bone.Rotation = GetJSONVec3(trns["rot"]);
				break;
			}
		}
	}
}

static void DoArmature()
{
	if (ImGui::Begin("Armature"))
	{
		if (debugArmature == nullptr)
		{
			ImGui::Text("No armature selected.");
		}
		else
		{
			ImGui::BeginChild("left pane", ImVec2(200, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			{
				int root = 0;
				for (auto i = 0; i < debugArmature->size(); i++)
				{
					if (debugArmature->at(i).Name == "Root")
					{
						root = i;
						break;
					}
				}
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 4);
				traverseArmature(root);
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			{
				auto& joint = debugArmature->at(selectedJoint);

				ImGui::Text(joint.Name.c_str());
				ImGui::Separator();

				ImGui::SeparatorText("Rotation");
				auto& tar = joint.Rotation;
				ImGui::DragFloat("X", &tar.x, 0.05f, -10, 10);
				ImGui::DragFloat("Y", &tar.y, 0.05f, -10, 10);
				ImGui::DragFloat("Z", &tar.z, 0.05f, -10, 10);
				if (ImGui::Button("Reset"))
					tar.x = tar.y = tar.z = 0;

				ImGui::SeparatorText("Inverse Bind Pose");
				auto& inv = joint.InverseBind;
				if (ImGui::BeginTable("invbind", 4))
				{
					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							ImGui::TableNextColumn();
							ImGui::Text("%.3f", inv[j][i]);
						}
					}
					ImGui::EndTable();
				}
			}

			if (ImGui::Button("Copy JSON"))
			{
				std::string json;
				json += "{\n";
				for (auto& bone : *debugArmature)
				{
					auto rot = bone.Rotation;
					if (rot.x + rot.y + rot.z == 0)
					{
						continue;
					}
					json += fmt::format("\t\"{}\": {{ \"rot\": [ {}, {}, {} ] }},\n", bone.Name, rot.x, rot.y, rot.z);
				}
				json = json.substr(0, json.length() - 2); //remove trailing ",\n"
				json += "\n}\n";
				ImGui::SetClipboardText(json.c_str());
			}
			ImGui::SameLine();
			if (ImGui::Button("Paste JSON"))
			{
				try
				{
					auto json = json5pp::parse5(ImGui::GetClipboardText());
					applyPose(json);
				}
				catch (std::runtime_error& x)
				{
					conprint(4, x.what());
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("A"))
			{
				try
				{
					auto json = json5pp::parse5(R"JSON({
						"Arm_2_L": { "rot": [ 0,  0.0,  0.5 ] },
						"Arm_1_L": { "rot": [ 0, -1.2, -0.4 ] },
						"Arm_2_R": { "rot": [ 0,  0.0,  0.5 ] },
						"Arm_1_R": { "rot": [ 0, -1.2, -0.4 ] }
					})JSON");
					applyPose(json);
				}
				catch (std::runtime_error& x)
				{
					conprint(4, x.what());
				}
			}

			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void Game::ImGui()
{
	DoCamera();
	DoLights();
	DoVillager();
	DoPlayer();
	DoArmature();
}
