#include "CameraActor.h"
#include <imgui.h>
#include <iostream>
#include "Components/CameraComponent.h"
#include "Game/Actor.h"
#include "Game/TimerManager.h"
#include "Math/LinearAlgebra.h"

CameraActor::CameraActor()
{
    Component = AddComponent<CameraComponent>().get();
}

void CameraActor::BeginPlay()
{
	Actor::BeginPlay();
	World->GetViewport()->MouseLeftButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseLeftDragRotation);
	World->GetViewport()->MouseRightButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseRightDragTranslation);
	World->GetViewport()->MouseScrollEvent.AddObjectFunction(this, &CameraActor::MouseWheelZoom);
}

void CameraActor::LookAt(const FVector& Target)
{
	FocusCenter = Target;
	SetRotation(FQuat{LinearAlgbera::LookAtMatrix(GetLocation(), FocusCenter)});
}

void CameraActor::LookAt()
{
	LookAt(FocusCenter);
}

void CameraActor::BlendeTo(const FTransform& TargetTransform, double Duration)
{
	GetWorld()->GetTimerManager()->AddTimer(Duration,
		[TargetTransform, this]() {
			FTransform CurrentTransform = GetFTransform();
			SetTransform(CurrentTransform.LerpTo(TargetTransform, 0.5));
		});
}

void CameraActor::BlendeTo(const FVector& TargetLocation, double Duration)
{
	GetWorld()->GetTimerManager()->AddTimer(Duration,
	[TargetLocation, this]() {
		auto CurrentLocation = GetLocation();
		auto Radius = (CurrentLocation - FocusCenter).norm();
		auto NextLocation = Lerp(CurrentLocation, TargetLocation, 0.1);
		NextLocation = (NextLocation - FocusCenter).normalized() * Radius + FocusCenter;
		SetTranslation(NextLocation);
		LookAt();
	});
}

void CameraActor::MouseLeftDragRotation(FVector2 StartPos, FVector2 Delta)
{
	static FVector2 LastPos = {0.f, 0.f};
	static FVector2 LastDelta = {0.f, 0.f};
	static FQuat LastRotation;
	if(StartPos == LastPos)
	{
		auto Delta = World->GetViewport()->ScreenToNDC(StartPos.cast<double>()) -
				World->GetViewport()->ScreenToNDC((StartPos + LastDelta).cast<double>());

		FQuat NewRotation = FQuat(Eigen::AngleAxisd(M_PI * Delta.x() * RotationSpeed, FVector{0,0,-1} )) * LastRotation;
		NewRotation = NewRotation * FQuat(Eigen::AngleAxisd(M_PI * Delta.y() * RotationSpeed, FVector{0,1,0}));
		NewRotation.normalize();
		SetRotation(NewRotation);

		auto Forward = (NewRotation * FVector{1, 0, 0}).normalized() * DistranceToTarget;
		SetTranslation(FocusCenter - Forward);
	}
	else {
		LastRotation = GetRotation();
		DistranceToTarget = (GetLocation() - FocusCenter).norm();
	}
	LastPos = StartPos;
	LastDelta = Delta;
}

void CameraActor::MouseRightDragTranslation(FVector2 StartPos, FVector2 Delta)
{
	static FVector2 LastPos = {0.f, 0.f};
	static double ClipSpaceZ = 0.;
	static FVector DownPos;
	static FVector PreCenter;

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


			FocusCenter = PreCenter - Delta;
			SetTranslation(DownPos - Delta);
		}
	}
	else {
		auto ClipSpace = Component->Project(FocusCenter);
		ClipSpaceZ = ClipSpace.z();
		DownPos = GetTranslation();
		PreCenter = FocusCenter;
	}
	LastPos = StartPos;
}

void CameraActor::MouseWheelZoom(FVector2 Delta)
{
	AddTranslationLocal({TranslationSpeed * 0.6f * Delta.x(), 0., 0.});
}