//
// Created by MarvelLi on 2024/1/15.
//

#pragma once
#include "UIWidget.h"
#include "Core/CoreMinimal.h"

class Actor;
class WorldOutliner : public UIWidget
{
public:
	explicit WorldOutliner() : UIWidget("WorldOutliner") {}
	virtual void Draw() override;
};
