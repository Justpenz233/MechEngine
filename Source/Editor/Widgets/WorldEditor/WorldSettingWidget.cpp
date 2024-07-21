//
// Created by Mayn on 2024/7/21.
//

#include "WorldSettingWidget.h"
#include "ImguiPlus.h"
#include "Core/GlobalSymbols.h"
#include "Game/World.h"

void WorldSettingWidget::Draw()
{
	ImGui::Begin(ICON_FA_CAMERA_RETRO "World Settings");
	ImGui::Text("Average FPS: %d", int(GAverageFPS));
	ImGui::Text("Average MS: %d ms", int(GAverageMS));
	ImGui::End();
}
