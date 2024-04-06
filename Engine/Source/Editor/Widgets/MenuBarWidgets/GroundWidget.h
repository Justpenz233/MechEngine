#pragma once
#include "Core/CoreMinimal.h"
#include "Imguizmo/Imguizmo.h"
#include "UIWidget.h"


class GroundWidget : public MenuBarItem
{
public:
    GroundWidget() : MenuBarItem("GroundWidget") {
        Visible = false;
    }

	virtual void OnClick() override
    {
	    ToggleVisible();
    }

    virtual void Draw() override
    {
        if(!Visible) return;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGuizmo::BeginFrame();
        ImGui::PopStyleVar();

        Eigen::Matrix4f view = World->GetScene()->GetViewMatrix().cast<float>();
        Eigen::Matrix4f proj = World->GetScene()->GetProjectionMatrix().cast<float>();
        // if (viewer->core().orthographic)
        // {
        //     view(2, 3) -= 1000; /* Hack depth */
        // }
        // ImGuizmo doesn't like a lot of scaling in view, shift it to T
        const float			  z = 1.;
        const Eigen::Matrix4f S =
            (Eigen::Matrix4f() << z, 0, 0, 0, 0, z, 0, 0, 0, 0, z, 0, 0, 0, 0, 1).finished();
        view = (view * S.inverse()).eval();
        Eigen::Matrix4f Ide = Eigen::Matrix4f::Identity();
    	Ide.row(1).swap(Ide.row(2));
        
	    ImGuiIO& io = ImGui::GetIO();
	    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::DrawGrid(view.data(), proj.data(), Ide.data(), 100.f);
    }
};