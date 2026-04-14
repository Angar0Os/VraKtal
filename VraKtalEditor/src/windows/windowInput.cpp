#pragma once
#include "../../include/windows/WindowInput.h"
#include <core/input/input.h>


#include <imGuizmo/ImGuizmo.h>
#include <imgui/imgui.h>

WindowInput::WindowInput(core::Input& _input) : m_input(_input){}

WindowInput::~WindowInput()
{
}

void WindowInput::Draw()
{
	ImGui::Begin("Inputs");

	if (ImGui::BeginTable("InputTable", 2))
	{
		ImGui::TableSetupColumn("Action");
		ImGui::TableSetupColumn("Key");
		ImGui::TableHeadersRow();

		auto& ActionsStorage	= m_input.GetActionsStorage();
		auto& AxisStorage		= m_input.GetAxisStorage();

#pragma region ActionStorage
		for (const auto& actionName : ActionsStorage.GetAllNames())
		{
			uint32_t actionId = ActionsStorage.Find(actionName);
			if (actionId == ActionsStorage.INVALID_ID)
				continue;

			auto& action = ActionsStorage.Get(actionId);
			std::vector<input::Key>& actionKeys = action.keys;

			ImGui::PushID(static_cast<int>(actionId));
			ImGui::TableNextRow();

			// Colonne Action
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(actionName.c_str());
			ImGui::PopID();

			// Colonne Key
			ImGui::TableNextColumn();
			if (bIsEditingInput && m_keyActionEditing == &action)
			{
				std::vector<int> keyPressed = m_input.GetCurrentKeyPressed();
				if (m_input.GetCurrentKeyPressed().size() == 0 && bStartedInput)
				{
					bStartedInput = false;
					bIsEditingInput = false;
					m_keyActionEditing->keys = m_keysRecoreded;
				}
				else if (keyPressed.size() > 0)
				{
					bStartedInput = true;
					bool alreadyInAction = false;

					for (int currentKey : keyPressed)
					{
						for (int recordedKey : m_keysRecoreded)
						{
							if (currentKey == recordedKey)
							{
								alreadyInAction = true;
								break;
							}
						}

						if (!alreadyInAction)
						{
							std::cout << "Added Key: " << input::KeyToString(input::Key(currentKey)) << std::endl;
							m_keysRecoreded.push_back(input::Key(currentKey));
						}
						alreadyInAction = false;
					}
				}

				if (InputButton("Cancel", ImVec2(-FLT_MIN, 0.0f)))
				{
					bIsEditingInput = false;
					m_keyActionEditing = nullptr;
				}
			}
			else
			{
				std::string keysText;
				for (size_t i = 0; i < actionKeys.size(); i++)
				{
					if (i > 0)
						keysText += " + ";

					keysText += input::KeyToString(actionKeys[i]);
				}
				if (InputButton(keysText.c_str(), ImVec2(-FLT_MIN, 0.0f)))
				{
					bIsEditingInput = true;
					m_keyActionEditing = &action;
					m_keyActionEditing->keys.clear();
					m_keysRecoreded.clear();
				}
			}
		}
		for (const auto& axisActionName : AxisStorage.GetAllNames())
		{
			uint32_t actionId = AxisStorage.Find(axisActionName);
			if (actionId == AxisStorage.INVALID_ID)
				continue;

			auto& AxisAction = AxisStorage.Get(actionId);

			std::string keysText;
			keysText += "X:";
			keysText += input::KeyToString(AxisAction.key[0]);
			keysText += " -X:";
			keysText += input::KeyToString(AxisAction.key[1]);
			keysText += " Y:";
			keysText += input::KeyToString(AxisAction.key[2]);
			keysText += " -Y:";
			keysText += input::KeyToString(AxisAction.key[3]);

			ImGui::PushID((int)actionId);
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(axisActionName.c_str());
			ImGui::TableNextColumn();
			if (InputButton(keysText.c_str(), ImVec2(-FLT_MIN, 0.0f)))
			{
				if (m_openedAxisAction == actionId)
				{
					m_openedAxisAction = -1;
				}
				else
				{
					m_openedAxisAction = actionId;
				}
			}
			if (m_openedAxisAction == actionId)
			{
				ImGui::Text(" X " , ImVec2(-FLT_MIN, 0.0f));
				ImGui::SameLine();
				if (InputButton(input::KeyToString(AxisAction.key[0]), ImVec2(-FLT_MIN, 0.0f)))
				{
					AxisToChange = 0;
					bIsEditingInput = true;
					m_axisActionEditing = &AxisAction;
				}
				ImGui::Text("-X ", ImVec2(-FLT_MIN, 0.0f));
				ImGui::SameLine();
				if (InputButton(input::KeyToString(AxisAction.key[1]), ImVec2(-FLT_MIN, 0.0f)))
				{
					AxisToChange = 1;
					bIsEditingInput = true;
					m_axisActionEditing = &AxisAction;
				}
				ImGui::Text(" Y ", ImVec2(-FLT_MIN, 0.0f));
				ImGui::SameLine();
				if(InputButton(input::KeyToString(AxisAction.key[2]), ImVec2(-FLT_MIN, 0.0f)))
				{
					AxisToChange = 2;
					bIsEditingInput = true;
					m_axisActionEditing = &AxisAction;
				}
				ImGui::Text("-Y ", ImVec2(-FLT_MIN, 0.0f));
				ImGui::SameLine();
				if (InputButton(input::KeyToString(AxisAction.key[3]), ImVec2(-FLT_MIN, 0.0f)))
				{
					AxisToChange = 3;
					bIsEditingInput = true;
					m_axisActionEditing = &AxisAction;
				}

			}

			if (bIsEditingInput && AxisToChange != -1 && m_axisActionEditing != nullptr)
			{
				auto currentKeys = m_input.GetCurrentKeyPressed();
				if (currentKeys.size() > 0)
				{
					m_axisActionEditing->key[AxisToChange] = input::Key(currentKeys[0]);
					bIsEditingInput = false;
					m_axisActionEditing = nullptr;
				}

			}


			ImGui::PopID();
		}
#pragma endregion

		ImGui::EndTable();
	}

	ImGui::End();
}

bool WindowInput::InputButton(const char* label, const ImVec2& size)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.28f, 0.28f, 1.00f));

	bool pressed = ImGui::Button(label, size);

	ImGui::PopStyleColor(3);
	return pressed;
}