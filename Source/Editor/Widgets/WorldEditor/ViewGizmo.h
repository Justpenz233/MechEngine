//
// Created by MarvelLi on 2024/5/5.
//

#pragma once
#include "UIWidget.h"
#include "Actors/CameraActor.h"
#include "Game/World.h"
#include "Imguizmo/Imguizmo.h"
#include "imgui.h"


class ViewGizmo : public UIWidget
{
public:
    ViewGizmo():  UIWidget("ViewGizmo") { Visible = true; }

    virtual void Draw() override
    {
        if(!Visible) return;
        // Add new
        if(!World) return;

        auto Camera = World->GetCurrentCamera();
        if(Camera == nullptr) return;

        Matrix4f ViewMatrix = Camera->GetCameraComponent()->GetViewMatrix().cast<float>();
        Matrix4f PreView = ViewMatrix;
        auto FocusCenter = Camera->GetFocusCenter();
        float Length = (Camera->GetTranslation() - FocusCenter).norm();

        auto WindowSize = ImGui::GetMainViewport()->WorkSize;
        auto WindowPos = ImGui::GetMainViewport()->WorkPos;

        // ImVec2 WorldOutlinerPos = {WindowPos.x + WindowSize.x - 300, WindowPos.y};
        ImVec2 Pos = {WindowPos.x + WindowSize.x - 400, WindowPos.y};
        ImVec2 Size = {100, 100};

        ImGuizmo::BeginFrame();
        ImGuizmo::ViewManipulate(ViewMatrix.data(), Length, Pos, Size, 0x10101010);

        if((PreView - ViewMatrix).maxCoeff() > 1e-2)
        {
            Matrix4d SwapMatrix;
            SwapMatrix
                <<  0, 1, 0, 0,
                    0, 0, 1, 0,
                    1, 0, 0, 0,
                    0, 0, 0, 1;

            Matrix4d TransfomInverse = SwapMatrix.transpose() * ViewMatrix.cast<double>();
            Affine3d NewTransform(TransfomInverse);
            Camera->SetTransform(NewTransform.inverse());
        }
    }
};
