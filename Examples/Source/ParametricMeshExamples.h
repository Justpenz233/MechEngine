//
// Created by MarvelLi on 2024/3/13.
//
#pragma once
#include <imgui.h>
#include "LambdaUIWidget.h"
#include "Actors/CameraActor.h"
#include "Actors/ParametricMeshActor.h"
#include "Game/StaticMeshActor.h"
#include "Game/World.h"
#include "Materials/Material.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Misc/Path.h"

#pragma once
inline auto ParametricMeshExamples()
{
    return [](World& World)
    {
        auto Camera = World.SpawnActor<CameraActor>("MainCamera");
        Camera->SetTranslation({-5, 0, 0});

        auto Surface = World.SpawnActor<ParametricMeshActor>("ParametrizationSurface", StaticMesh::LoadFromObj(Path("openbunny.obj")));
        auto BunnyUVIndicator = World.SpawnActor<StaticMeshActor>("BunnyUVIndicator", BasicShapesLibrary::GenerateSphere(0.003, 16));
        BunnyUVIndicator->GetStaticMeshComponent()->GetMeshData()->GetMaterial()->BaseColor = {1, 0, 0};
        World.AddWidget<LambdaUIWidget>([Surface, BunnyUVIndicator]() {
            ImGui::Begin("Parametrization Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            static float UV[2] = {0, 0};
            ImGui::DragFloat2("Bunny UV", UV, 0.01, 0., 1.);
            if (Surface->ValidUV(UV[0], UV[1]))
            {
                auto P = Surface->Sample(UV[0], UV[1]);
                BunnyUVIndicator->SetTranslation(P);
                ImGui::Text("Pos: %lf ,%lf ,%lf", P[0], P[1], P[2]);
            }
            else
            {
                ImGui::Text("UV invalid!");
            }
            ImGui::End();
        });

        auto Surface2 = World.SpawnActor<ParametricMeshActor>("ParametrizationSurface", "Catenoid"); Surface2->SetScale({0.1, 0.1, 0.1});
        auto CatenoidUVIndicator = World.SpawnActor<StaticMeshActor>("CatenoidUVIndicator", BasicShapesLibrary::GenerateSphere(0.005, 16));
        CatenoidUVIndicator->GetStaticMeshComponent()->GetMeshData()->GetMaterial()->BaseColor = {1, 0, 0};

        Surface2->SetTranslation({0.5, 0, 0});
        World.AddWidget<LambdaUIWidget>([Surface2, CatenoidUVIndicator]() {
            ImGui::Begin("Parametrization Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            static float UV[2] = {0, 0};
            ImGui::DragFloat2(" Catenoid UV", UV, 0.01, 0., 1.);
            if (Surface2->ValidUV(UV[0], UV[1]))
            {
                auto P = Surface2->Sample(UV[0], UV[1]);
                CatenoidUVIndicator->SetTranslation(P);
                ImGui::Text("Pos: %lf ,%lf ,%lf", P[0], P[1], P[2]);
            }
            else
            {
                ImGui::Text("UV invalid!");
            }
            ImGui::End();
        });
    };
}