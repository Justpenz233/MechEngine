//
// Created by MarvelLi on 2024/3/29.
//

#pragma once

#include "ImguiPlus.h"
#include "Render/ViewportInterface.h"
#include "luisa/gui/imgui_window.h"
#include "IconsFontAwesome6.h"
#include "Misc/Config.h"

#include <iostream>

class LuisaViewport : public ViewportInterface
{
public:

	float CurrentDpiScaling = 1.0f;
	float DefaultFontSize = GConfig.Get<float>("Editor", "FontSize");

	LuisaViewport(uint InWidth, uint InHeight, RenderPipeline* InRenderer,
		luisa::compute::ImGuiWindow* InMainWindow, luisa::compute::Stream& InStream, luisa::compute::Device& InDevice)
		: ViewportInterface(InWidth, InHeight, InRenderer), Stream(InStream), Device(InDevice)
	{
		MainWindow = InMainWindow;
	}

	virtual std::pair<uint, uint> GetWindowSize() const override
	{
		auto Size = ImGui::GetMainViewport()->Size;
		return {static_cast<uint>(Size.x), static_cast<uint>(Size.y)};
	}

	virtual float GetAspectRatio() const override
	{
		auto Size = ImGui::GetMainViewport()->Size;
		return Size.x / Size.y;
	}

	float GetHidpiScaling()
	{
		auto Scale =  ImGui::GetIO().DisplayFramebufferScale;
		return (Scale.x + Scale.y) * 0.5f;
	}

	virtual void LoadViewportStyle() override
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		ImGui::StyleColorsDark();

		auto LayoutFilePaht = Path::ProjectConfigDir() / "EditorLayout.config";
		if(!exists(LayoutFilePaht))
		{
			ImGui::LoadIniSettingsFromDisk((Path::EngineConfigDir() / "DefaultEditorLayout.config").string().c_str());
		}
		else
		{
			ImGui::LoadIniSettingsFromDisk(LayoutFilePaht.string().c_str());
		}
		char* copy = new char[LayoutFilePaht.string().length() + 1];
		strcpy(copy, LayoutFilePaht.string().c_str());
		ImGui::GetIO().IniFilename = copy;

		// Set Style
		ImGuiStyle * style = &ImGui::GetStyle();
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f); //Center
		style->WindowRounding = 0.0f;
		style->FrameRounding = 0.0f;

		ReloadFont();
	}

	virtual void PreFrame() override
	{
		if(auto DpiScale = GetHidpiScaling();std::abs(CurrentDpiScaling - DpiScale) > 1e-5)
		{
			ReloadFont();
		}
		auto dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	}

protected:
	luisa::compute::Stream&      Stream;
	luisa::compute::Device&      Device;
	luisa::compute::ImGuiWindow* MainWindow = nullptr;
	luisa::compute::Image<float> FontTexture;

	void ReloadFont()
	{
		auto DpiScale = GetHidpiScaling();
		CurrentDpiScaling = DpiScale;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();

		float BaseFontSize = DefaultFontSize * DpiScale;
		// @see https://github.com/juliettef/IconFontCppHeaders
		float IconFontSize = BaseFontSize / 1.5f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = IconFontSize;
		std::filesystem::path DefaultFont = std::filesystem::path("Font") / "Cousine-Regular.ttf";
		std::filesystem::path IconFont = std::filesystem::path("Font") / "fa-solid-900.ttf";
		DefaultFont = Path::EngineContentDir() / DefaultFont;
		IconFont = Path::EngineContentDir() / IconFont;
		io.Fonts->AddFontFromFileTTF(DefaultFont.string().c_str(), BaseFontSize);
		io.Fonts->AddFontFromFileTTF(IconFont.string().c_str(), IconFontSize, &icons_config, icons_ranges);
		io.FontGlobalScale = 1.0 / DpiScale;

		auto pixels = static_cast<unsigned char *>(nullptr);
		auto width = 0, height = 0;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		if(!FontTexture || FontTexture.size().x != width || FontTexture.size().y != height)
		{
			FontTexture = Device.create_image<float>(luisa::compute::PixelStorage::BYTE4, width, height, 1);
			LOG_WARNING("Reload Font Texture: {}x{}", width, height);
		}

		Stream << FontTexture.copy_from(pixels) << luisa::compute::synchronize();
		auto tex_id = MainWindow->register_texture(FontTexture, luisa::compute::Sampler::linear_point_edge());
		io.Fonts->SetTexID(ImTextureID(tex_id));
	}
};