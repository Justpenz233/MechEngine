#pragma once
#include "CoreMinimal.h"
#include "UI/Imguizmo/Imguizmo.h"
#include "UI/UIWidget.h"


class TransformGizmo : public UIWidget
{
public:
    TransformGizmo():  UIWidget("GizoManager") { Visible = false; }

    virtual void Draw() override;

protected:
	std::map<class Actor*, int> GizemoID;

	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

	int LastUsing = -1;
	int TotalId = 0;
};