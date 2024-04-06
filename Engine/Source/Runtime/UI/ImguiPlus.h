//
// Created by MarvelLi on 2024/1/27.
//

#pragma once
#include "imgui.h"
#include "Math/MathType.h"

namespace ImGui
{
	void CenteredText(const char* label, const float& TextHeight = 20.);

	// If key is pressed in the application (not in the ImGui window)
	// was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
	bool IsKeyPressedApp(ImGuiKey key, bool repeat = true);

	bool IsMousePressedApp(int button);

	// Get the mouse wheel delta from the pre frame
	// 0 for left, 1 for right, 2 for middle
	// @return: {x, y} for the delta
	FVector2 GetMouseDragDeltaFrame(int button);


	bool  InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

} // namespace ImGui
