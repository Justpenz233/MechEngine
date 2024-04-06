#pragma once
#include "Core/CoreMinimal.h"
#include "Object/Object.h"

class UIWidget : public Object
{
public:
	explicit UIWidget(const String& Name)
    {
	    ObjectName = Name;
    }
    
    virtual void Enable() {}

    virtual void Disable() {}

    virtual void ToggleVisible() { Visible = !Visible;}

    virtual void SetVisible(bool InVisible) { Visible = InVisible; }

	template<class T, class... Args>
	T* AddSubWidget(Args&&... args);

    virtual void Draw() = 0;
    
    friend class World;

protected:
    bool Visible = true;

    /**
     * Some widgets may have sub widgets, like a menu bar may have menu items
     */
    TArray<UniqueObjectPtr<UIWidget>> SubWidgets;

	// This filed will be set by the World when calling AddWidget
    class World* World = nullptr;
};

template <class T>
concept IsUIWidget = std::is_base_of_v<UIWidget, T>;


template<class T, class... Args>
T* UIWidget::AddSubWidget(Args&&... args)
{
	static_assert(IsUIWidget<T>, "T must be a UI Widget");
	static_assert(std::is_constructible_v<T, Args...>, "T must be constructible with Args");

	auto NewWidget = MakeUnique<T>(std::forward<Args>(args)...);
	NewWidget->World = World;
	T* NewWidgetPtr = NewWidget.get();
	SubWidgets.push_back(std::move(NewWidget));
	return NewWidgetPtr;
}