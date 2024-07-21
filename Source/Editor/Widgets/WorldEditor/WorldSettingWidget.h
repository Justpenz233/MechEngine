//
// Created by Mayn on 2024/7/21.
//

#pragma once
#include "UIWidget.h"

class WorldSettingWidget : public UIWidget
{
public:
	WorldSettingWidget() : UIWidget("WorldSettingWidget") {};

	void Draw() override;
};
