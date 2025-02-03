//
// Created by MarvelLi on 2024/4/1.
//

#include "ViewportInterface.h"
#include "ImguiPlus.h"

FVector2 ViewportInterface::ScreenToViewport(const FVector2& ScreenPos) const
{
	return ScreenPos - FVector2{ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y};
}

FVector2 ViewportInterface::ViewportToNDC(const FVector2& ViewportPos) const
{
	return {(ViewportPos.x() / ImGui::GetMainViewport()->Size.x - 0.5) * 2,
			(0.5 - ViewportPos.y() / ImGui::GetMainViewport()->Size.y) * 2};
}

void ViewportInterface::HandleMouseInput()
{
	if(!ImGui::GetIO().WantCaptureMouse)
	{
		if(ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			auto [Dx, Dy] = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
			auto [Sx, Sy] = ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Left];
			MouseLeftButtonDragEvent.Broadcast({Sx, Sy}, {Dx, Dy});
		}
		if(ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			auto [Dx, Dy] = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			auto [Sx, Sy] = ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Right];
			MouseRightButtonDragEvent.Broadcast({Sx, Sy}, {Dx, Dy});
		}
		if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			ImVec2 MousePosScreen = ImGui::GetMousePos();
			ImVec2 WinPos = ImGui::GetMainViewport()->Pos;
			ImVec2 RelativePos = ImVec2(MousePosScreen.x - WinPos.x, MousePosScreen.y - WinPos.y);
			MouseLeftButtonDownEvent.Broadcast({RelativePos.x, RelativePos.y});
		}
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			ImVec2 MousePosScreen = ImGui::GetMousePos();
			ImVec2 WinPos = ImGui::GetMainViewport()->Pos;
			ImVec2 RelativePos = ImVec2(MousePosScreen.x - WinPos.x, MousePosScreen.y - WinPos.y);
			MouseLeftButtonUpEvent.Broadcast({RelativePos.x, RelativePos.y});
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseDragPastThreshold(ImGuiMouseButton_Left))
		{
			ImVec2 MousePosScreen = ImGui::GetMousePos();
			ImVec2 WinPos = ImGui::GetMainViewport()->Pos;
			ImVec2 RelativePos = ImVec2(MousePosScreen.x - WinPos.x, MousePosScreen.y - WinPos.y);
			MouseLeftButtonClickedEvent.Broadcast({RelativePos.x, RelativePos.y});
		}
		// Mouse Scroll
		if(ImGui::GetIO().MouseWheel != 0 || ImGui::GetIO().MouseWheelH != 0)
		{
			MouseScrollEvent.Broadcast({ImGui::GetIO().MouseWheel, ImGui::GetIO().MouseWheelH});
		}
	}
}

void ViewportInterface::HandleKeyboardInput()
{
	if(!ImGui::GetIO().WantCaptureKeyboard)
	{
		for(int i = ImGuiKey_NamedKey_BEGIN;i < ImGuiKey_NamedKey_END; i ++)
		{
			if(ImGui::IsKeyPressed(static_cast<ImGuiKey>(i)))
			{
				KeyPressedEvent.Broadcast(i);
			}
		}
	}
}