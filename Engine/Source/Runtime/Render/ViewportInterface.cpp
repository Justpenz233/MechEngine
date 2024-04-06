//
// Created by MarvelLi on 2024/4/1.
//

#include "ViewportInterface.h"

#include "ImguiPlus.h"

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
	}
}

void ViewportInterface::HandleKeyboardInput()
{
	if(!ImGui::GetIO().WantCaptureKeyboard)
	{

	}
}