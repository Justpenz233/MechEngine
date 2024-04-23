#include "CameraActor.h"

#include <imgui.h>
#include <iostream>

#include "Components/CameraComponent.h"
#include "Game/Actor.h"

CameraActor::CameraActor() : Actor()
{
    Component = AddComponent<CameraComponent>().get();
}

void CameraActor::BeginPlay()
{
	Actor::BeginPlay();
	World->GetViewport()->MouseLeftButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseLeftDragRotation);
	World->GetViewport()->MouseRightButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseRightDragTranslation);
	World->GetViewport()->MouseScrollEvent.AddObjectFunction(this, &CameraActor::MouseWheelZoom);
	World->GetViewport()->KeyPressedEvent.AddObjectFunction(this, &CameraActor::KeyPressedEvent);
}

void CameraActor::MouseLeftDragRotation(FVector2 StartPos, FVector2 Delta)
{
	static FVector2 LastPos = {0.f, 0.f};
	static FVector2 LastDelta = {0.f, 0.f};
	static FQuat LastRotation;
	static double R;
	if(StartPos == LastPos)
	{
		if(auto Focus = World->GetSelectedActor()) {
			auto Delta = World->GetViewport()->ScreenToNDC(StartPos.cast<double>()) -
				World->GetViewport()->ScreenToNDC((StartPos + LastDelta).cast<double>());

			FQuat NewRotation = FQuat(Eigen::AngleAxisd(M_PI * Delta.x(), FVector{0,0,-1} )) * LastRotation;
			NewRotation = NewRotation * FQuat(Eigen::AngleAxisd(M_PI * Delta.y(), FVector{0,1,0}));
			NewRotation.normalize();
			SetRotation(NewRotation);

			auto Forward = (GetRotation() * FVector{1, 0, 0}).normalized() * R;
			SetTranslation(Focus->GetTranslation() - Forward);
		}

	}
	else {
		LastRotation = GetRotation();
		if (auto Focus = World->GetSelectedActor()) {
			R = (GetLocation() - World->GetSelectedActor()->GetTranslation()).norm();
		}
	}
	LastPos = StartPos;
	LastDelta = Delta;
}

void CameraActor::MouseRightDragTranslation(FVector2 StartPos, FVector2 Delta)
{
	static FVector2 LastPos = {0.f, 0.f};
	static double ClipSpaceZ = 0.;
	static FVector DownPos;

	if(StartPos == LastPos)
	{
		if (auto Focus = World->GetSelectedActor())
		{
			auto NowMouse = StartPos + Delta;

			// Mouse down world position
			auto NDCSpace0 = World->GetViewport()->ScreenToNDC(StartPos.cast<double>());
			auto UnProject0 = Component->UnProject({NDCSpace0.x(), NDCSpace0.y(), ClipSpaceZ});

			// Mouse now world position
			auto NDCSpace1 = World->GetViewport()->ScreenToNDC(NowMouse.cast<double>());
			auto UnProject1 = Component->UnProject({NDCSpace1.x(), NDCSpace1.y(), ClipSpaceZ});

			auto Delta = UnProject1 - UnProject0;
			SetTranslation(DownPos - Delta);
		}
	}
	else {
		if (auto Focus = World->GetSelectedActor()) {
			auto ClipSpace = Component->Project(Focus->GetTranslation());
			ClipSpaceZ = ClipSpace.z();
			DownPos = GetTranslation();
		}
	}
	LastPos = StartPos;
}

void CameraActor::MouseWheelZoom(FVector2 Delta)
{
	AddTranslationLocal({TranslationSpeed * Delta.x(), 0., 0.});
}

void CameraActor::KeyPressedEvent(int Key)
{
	if (Key == ImGuiKey_W) // W
		AddTranslationLocal({TranslationSpeed, 0., 0.});
	else if (Key == ImGuiKey_S) // S
		AddTranslationLocal({-TranslationSpeed, 0., 0.});
	else if (Key == ImGuiKey_A) // A
		AddTranslationLocal({0., -TranslationSpeed, 0.});
	else if (Key == ImGuiKey_D) // D
		AddTranslationLocal({0., TranslationSpeed, 0.});
	else if (Key == ImGuiKey_Space) // Space
		AddTranslationLocal({0., 0., TranslationSpeed});
	else if (Key == ImGuiKey_C) // C
		AddTranslationLocal({0., 0., -TranslationSpeed});
}
