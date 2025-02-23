#include "CameraActor.h"
#include <imgui.h>
#include "Components/CameraComponent.h"
#include "Game/Actor.h"
#include "Game/TimerManager.h"
#include "Math/LinearAlgebra.h"
#include "Render/Core/RayCastHit.h"

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
	World->GetViewport()->MouseLeftButtonClickedEvent.AddLambda([this](FVector2 MousePos) {
		auto HitObjectId = GetWorld()->RayCastQuery(MousePos.x() * 2., MousePos.y() * 2.);
		if(!HitObjectId.miss())
			GetWorld()->SelectActorByInstanceId(HitObjectId.instance_id);
	});
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

void CameraActor::BlendTo(const FTransform& TargetTransform, double Duration)
{
	GetWorld()->GetTimerManager()->AddTimer(Duration,
		[TargetTransform, this]() {
			FTransform CurrentTransform = GetFTransform();
			SetTransform(CurrentTransform.LerpTo(TargetTransform, 0.5));
		});
}

void CameraActor::BlendTo(const FVector& TargetLocation, double Duration)
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
		FVector2 DeltaNDC = World->GetViewport()->ScreenToNDC(StartPos.cast<double>()) -
				World->GetViewport()->ScreenToNDC((StartPos + Delta).cast<double>());

		FQuat NewRotation = FQuat(Eigen::AngleAxisd(M_PI * DeltaNDC.x() * RotationSpeed, FVector{0,0,-1} )) * LastRotation;
		NewRotation = NewRotation * FQuat(Eigen::AngleAxisd(M_PI * DeltaNDC.y() * RotationSpeed, FVector{0,1,0}));
		NewRotation.normalize();
		SetRotation(NewRotation);

		FVector Forward = ((NewRotation * FVector{1, 0, 0}).normalized() * DistanceToTarget).eval();
		SetTranslation(FocusCenter - Forward);
	}
	else {
		LastRotation = GetRotation();
		DistanceToTarget = (GetLocation() - FocusCenter).norm();
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
	if(Delta.x() != 0)
		AddTranslationLocal({TranslationSpeed * 0.6f * Delta.x(), 0., 0.});
	if(Delta.y() != 0)
	{
		auto Rotation = AngleAxisd(Delta.y() * RotationSpeed * 0.05f, FVector{0, 0, 1});
		auto NewPos = Rotation * (GetLocation() - FocusCenter) + FocusCenter;
		SetTranslation(NewPos);
		LookAt();
	}

	DistanceToTarget = (GetLocation() - FocusCenter).norm();
}