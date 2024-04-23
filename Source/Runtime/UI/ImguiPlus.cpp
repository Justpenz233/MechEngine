//
// Created by MarvelLi on 2024/1/27.
//

#include "ImguiPlus.h"
#include "imgui_internal.h"
#include "Log/Log.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"    // warning: implicit conversion changes signedness
#endif

void ImGui::CenteredText(const char* label, const float& TextHeight)
{
	ImGuiContext&	  g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImGuiWindow*	  window = GetCurrentWindow();

	float margin = (style.WindowPadding.x) * 2.0f;
	float w = ImGui::GetWindowWidth() - margin;
	auto  size_arg = ImVec2(w, TextHeight);

	const ImGuiID id = window->GetID(label);
	const ImVec2  label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImVec2 pos2 = ImVec2((pos.x + size.x), (pos.y + size.y));
	const ImRect bb(pos, pos2);

	ItemSize(size, style.FramePadding.y);

	const ImVec2 pos_min = ImVec2((bb.Min.x + style.FramePadding.x), (bb.Min.y + style.FramePadding.y));
	const ImVec2 pos_max = ImVec2((bb.Max.x - style.FramePadding.x), (bb.Max.y - style.FramePadding.y));

	RenderTextClipped(pos_min, pos_max, label, NULL, &label_size, style.ButtonTextAlign, &bb);
}

bool ImGui::IsKeyPressedApp(ImGuiKey key, bool repeat)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return false;
	return ImGui::IsKeyPressed(key, repeat);
}

FVector2 ImGui::GetMouseDragDeltaFrame(int button)
{
	MechEngine::ASSERT(button < 3);
	static FVector2 last[3] = {{0, 0}, {0, 0}, {0, 0}};
	static int last_frame[3] = {0, 0, 0};
	if(ImGui::IsMouseDragging(button))
	{
		int CurrentFrame = ImGui::GetFrameCount();
		auto t = ImGui::GetMouseDragDelta(button);
		auto current = FVector2(t.x, t.y);
		auto result = FVector2{0., 0.};
		if(CurrentFrame == last_frame[button] + 1)
			result = current - last[button];
		last_frame[button] = CurrentFrame;
		last[button] = current;
		return result;
	}
	return {0, 0};
}


struct InputTextCallback_UserData
{
    std::string*            Str;
    ImGuiInputTextCallback  ChainCallback;
    void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
