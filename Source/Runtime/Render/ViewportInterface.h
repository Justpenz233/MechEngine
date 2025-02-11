//
// Created by MarvelLi on 2024/3/25.
//

#pragma once

#include "CoreMinimal.h"
#include "UIWidget.h"
#include "Delegate.h"

class RenderPipeline;
class UIWidget;
class World;

DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseLeftButtonDrag, FVector2, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseLeftButtonDown, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseLeftButtonUp, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseLeftButtonClicked, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseRightButtonDrag, FVector2, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnMouseScroll, FVector2);
DECLARE_MULTICAST_DELEGATE_PARMS(OnKeyPressed, int);
/**
 * An interface for a viewport.
 * Viewport is belong to the renderer. It should be responsible for rendering the UI and handle the input event.
 * As the interface to acess UI framework, currently only support ImGui
 * Viewport does not have the ownership of the UIWidget (only render it), World has the ownership of the UIWidget.
 */
class ViewportInterface
{
public:
	ViewportInterface(uint InWidth, uint InHeight, RenderPipeline* InRenderer)
	{ InitScreenWidth = InWidth; InitScreenHeight = InHeight; Renderer = InRenderer; }

	virtual ~ViewportInterface() = default;


	/**
	 * Load and set the style of the viewport
	 */
	virtual void LoadViewportStyle() = 0;

	/**
	 * The window size of the viewport
	 * @return Get the window size of this viewport
	 */
	[[nodiscard]] virtual std::pair<uint, uint> GetWindowSize() const = 0;

	/**
	 * Calculate the work size, aka render size, of the viewport
	 * @return the work size of the viewport
	 */
	// [[nodiscard]] virtual std::pair<uint, uint> GetWorkSpaceSize() const = 0;

	/**
	 * The aspect ratio of the viewport
	 * @return the aspect ratio of the viewport
	 */
	[[nodiscard]] virtual float GetAspectRatio() const = 0;

	/**
	 * Get the aspect ratio of the workspace
	 * @return the aspect ratio of the workspace
	 */
	// [[nodiscard]] virtual float GetWorkSpaceAspectRatio() const = 0;

	/**
	 * Called before new frame
	 */
	virtual void PreFrame() { }

	/**
	 * Called after new frame but before draw widgets
	 */
	virtual void PreDrawWidgets(){ }

	virtual void DrawWidgets()
	{
		PreDrawWidgets();
		for (const auto& Widget : Widgets)
		{
			if (!Widget.expired())
			{
				Widget.lock()->Draw();
			}
		}
		PostDrawWidgets();
	}

	/**
	 * Called after draw widgets before end frame
	 */
	virtual void PostDrawWidgets(){ }

	/**
	 * Called after end frame
	 */
	virtual void PostFrame() { }




public:
	/**
	 * Calculate the position relative to the viewport(app window), measured in pixels
	 * @param ScreenPos The mouse position in screen(monitor) space in pixels
	 * @return The position in viewport space
	 */
	[[nodiscard]] virtual FVector2 ScreenToViewport(const FVector2& ScreenPos) const;


	/**
	 * Convert viewport position to normalized device coordinates(NDC) space, which is the range of [-1, 1]^2
	 * Not clamped if the viewport position is out of the viewport
	 * @param ViewportPos The position in viewport space
	 * @return The position in NDC space
	 */
	[[nodiscard]] virtual FVector2 ViewportToNDC(const FVector2& ViewportPos) const;

	/**
	 * Convert screen position to normalized device coordinates(NDC) space, which is the range of [-1, 1]^2
	 * Not clamp if the screen position is out of the viewport
	 * @param ScreenPos The mouse position in screen(monitor) space in pixels
	 * @return The position in NDC space
	 */
	[[nodiscard]] virtual FVector2 ScreenToNDC(const FVector2& ScreenPos) const
	{
		return ViewportToNDC(ScreenToViewport(ScreenPos));
	}


	virtual void HandleInput()
	{
		HandleMouseInput();
		HandleKeyboardInput();
	}

	virtual void AddWidget(const WeakObjectPtr<UIWidget>& Widget)
	{
		Widgets.push_back(Widget);
	}

	OnMouseLeftButtonDrag MouseLeftButtonDragEvent;
	OnMouseRightButtonDrag MouseRightButtonDragEvent;
	OnMouseLeftButtonDown MouseLeftButtonDownEvent;
	OnMouseLeftButtonUp MouseLeftButtonUpEvent;
	OnMouseLeftButtonClicked MouseLeftButtonClickedEvent;
	OnMouseScroll MouseScrollEvent;
	OnKeyPressed KeyPressedEvent;
protected:
	/** Dragging threshold for mouse dragging */
	static constexpr float DraggingThreshold = 1.4f;

	virtual void HandleMouseInput();
	virtual void HandleKeyboardInput();
	uint InitScreenWidth;
	uint InitScreenHeight;
	class RenderPipeline* Renderer;
	TArray<WeakObjectPtr<UIWidget>> Widgets;
};