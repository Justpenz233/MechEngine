// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2022 Alec Jacobson <alecjacobson@gmail.com>
// Copyright (C) 2018 Jérémie Dumas <jeremie.dumas@ens-lyon.org>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
////////////////////////////////////////////////////////////////////////////////
#include "ImguiPlugin.h"
#include "IconsFontAwesome6.h"
#include "Misc/Path.h"
#include "igl/opengl/glfw/Viewer.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

void ImGuiPlugin::reload_font(int font_size)
{
	hidpi_scaling_ = hidpi_scaling();
	pixel_ratio_ = pixel_ratio();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	float BaseFontSize = font_size * hidpi_scaling_;
	// @see https://github.com/juliettef/IconFontCppHeaders
	float IconFontSize = BaseFontSize; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

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
	io.FontGlobalScale = 1.0 / pixel_ratio_;
}

void ImGuiPlugin::init(igl::opengl::glfw::Viewer* _viewer)
{
	ViewerPlugin::init(_viewer);
	// Setup ImGui binding
	if (_viewer)
	{
        LOG_INFO("Editor Init");

		IMGUI_CHECKVERSION();
		if (!context_)
		{
			// Single global context by default, but can be overridden by the user
			static ImGuiContext* __global_context = ImGui::CreateContext();
			context_ = __global_context;
		}
		const char* glsl_version = "#version 150";
		ImGui_ImplGlfw_InitForOpenGL(viewer->window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);
		ImGui::GetIO().IniFilename = nullptr;
		reload_font();


		ImGuiStyle * style = &ImGui::GetStyle();
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f); //Center
	}
}

void ImGuiPlugin::shutdown()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	// User is responsible for destroying context if a custom context is given
	// ImGui::DestroyContext(*context_);
}

bool ImGuiPlugin::pre_draw()
{
	glfwPollEvents();

	float scaling = hidpi_scaling();
	if (std::abs(scaling - hidpi_scaling_) > 1e-5)
	{
		reload_font();
		ImGui_ImplOpenGL3_DestroyDeviceObjects();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return false;
}

bool ImGuiPlugin::post_draw()
{
	for (auto& widget : Widgets)
	{
		if(!widget.expired())
		{
			widget.lock()->Draw();
		}
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return false;
}

void ImGuiPlugin::post_resize(int width, int height)
{
	if (context_)
	{
		ImGui::GetIO().DisplaySize.x = float(width);
		ImGui::GetIO().DisplaySize.y = float(height);
	}
}

float ImGuiPlugin::pixel_ratio()
{
	// Computes pixel ratio for hidpi devices
	int			buf_size[2];
	int			win_size[2];
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
	glfwGetWindowSize(window, &win_size[0], &win_size[1]);
	return (float)buf_size[0] / (float)win_size[0];
}

float ImGuiPlugin::hidpi_scaling()
{
	// Computes scaling factor for hidpi devices
	float xscale, yscale;
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetWindowContentScale(window, &xscale, &yscale);
	return 0.5 * (xscale + yscale);
}

std::pair<uint, uint> ImGuiPlugin::GetWindowSize() const
{
	return { static_cast<uint>(ImGui::GetIO().DisplaySize.x), static_cast<uint>(ImGui::GetIO().DisplaySize.y) };
}

float ImGuiPlugin::GetAspectRatio() const
{
	return ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y;
}

// Mouse IO
bool ImGuiPlugin::mouse_down(int button, int modifier)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return true;
	}
	// for( auto & widget : Widgets)
	// {
	//   if(widget->mouse_down(button, modifier)) { return true; }
	// }
	return false;
}

bool ImGuiPlugin::mouse_up(int button, int modifier)
{
	if( ImGui::GetIO().WantCaptureMouse) return true;
	//  !! Should not steal mouse up
	//  for( auto & widget : Widgets)
	//  {
	//    widget->mouse_up(button, modifier);
	//  }
	return false;
}

bool ImGuiPlugin::mouse_move(int mouse_x, int mouse_y)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return true;
	}
	// for( auto & widget : Widgets)
	// {
	//   if(widget->mouse_move(mouse_x, mouse_y)) { return true; }
	// }
	return false;
}

bool ImGuiPlugin::mouse_scroll(float delta_y)
{
	return ImGui::GetIO().WantCaptureMouse;
}

// Keyboard IO
bool ImGuiPlugin::key_pressed(unsigned int key, int modifiers)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return true;
	}
	// for(auto & widget : Widgets)
	// {
	//   if(widget->key_pressed(key,modifiers)) {return true; }
	// }
	return false;
}

bool ImGuiPlugin::key_down(int key, int modifiers)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return true;
	}
	// for(auto & widget : Widgets)
	// {
	//   if(widget->key_down(key,modifiers)) {return true; }
	// }
	return false;
}

bool ImGuiPlugin::key_up(int key, int modifiers)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return true;
	}
	// for(auto & widget : Widgets)
	// {
	//   if(widget->key_up(key,modifiers)) { return true; }
	// }
	return false;
}
