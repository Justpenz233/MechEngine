//
// Created by MarvelLi on 2024/1/27.
//


/***************************************************************************
 * This file contains extensions to the ImGui library.
 * i.e.:
 * functions with std stl
 * Notify widget
 * Input handling
 ***************************************************************************/

#pragma once
#include "ImguiNotify/ImGuiNotify.hpp"
#include "Math/MathType.h"
#include "Misc/Platform.h"

namespace ImGui
{
	ENGINE_API void CenteredText(const char* label, const float& TextHeight = 20.);

	// If key is pressed in the application (not in the ImGui window)
	// was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
	ENGINE_API bool IsKeyPressedApp(ImGuiKey key, bool repeat = true);

	ENGINE_API bool IsMousePressedApp(int button);

	// Get the mouse wheel delta from the pre frame
	// 0 for left, 1 for right, 2 for middle
	// @return: {x, y} for the delta
	ENGINE_API FVector2 GetMouseDragDeltaFrame(int button);

	inline void NotifyInfo(const std::string& message, const std::string& titie = "Info", int durationMS = 3000)
	{
		ImGuiToast messageToast(ImGuiToastType::Info, durationMS);
		messageToast.setTitle(titie.c_str());
		messageToast.setContent(message.c_str());
		ImGui::InsertNotification(messageToast);
	}

	inline void NotifyWarning(const std::string& message, const std::string& titie = "Warning", int durationMS = 3000)
	{
		ImGuiToast messageToast(ImGuiToastType::Warning, durationMS);
		messageToast.setTitle(titie.c_str());
		messageToast.setContent(message.c_str());
		ImGui::InsertNotification(messageToast);
	}

	inline void NotifyError(const std::string& message, const std::string& titie = "Error", int durationMS = 3000)
	{
		ImGuiToast messageToast(ImGuiToastType::Error, durationMS);
		messageToast.setTitle(titie.c_str());
		messageToast.setContent(message.c_str());
		ImGui::InsertNotification(messageToast);
	}

	inline void NotifySuccess(const std::string& message, const std::string& titie = "Success", int durationMS = 3000)
	{
		ImGuiToast messageToast(ImGuiToastType::Success, durationMS);
		messageToast.setTitle(titie.c_str());
		messageToast.setContent(message.c_str());
		ImGui::InsertNotification(messageToast);
	}

	ENGINE_API bool  InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	ENGINE_API bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	ENGINE_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

} // namespace ImGui
