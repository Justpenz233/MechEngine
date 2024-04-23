#pragma once
#include "MenuBarItem.h"
#include "Core/CoreMinimal.h"
#include "Core/Delegate.h"
#include "UI/UIWidget.h"

DECLARE_DELEGATE(OnClick); 

struct MenuBarTreeNode
{
    String Label;
    String Tips;
    OnClick OnClickDelegate;

    MenuBarTreeNode() = default;
    ~ MenuBarTreeNode();

    MenuBarTreeNode(const String& InLabel, const String& InTips, OnClick InOnClickDelegate)
    :Label(InLabel), Tips(InTips), OnClickDelegate(InOnClickDelegate)
     {};

    MenuBarTreeNode(TArray<String> InLabel, String InTips, OnClick InOnClickDelegate);

    TArray<MenuBarTreeNode*> ChildItem;
    
    bool IsItem() const { return ChildItem.empty(); }
    void Draw();
    void AddItem(TArray<String> LablePath, String Tips, OnClick OnClickDelegate);
};

struct MenuBarTreeRoot
{
    ~MenuBarTreeRoot();
    TArray<MenuBarTreeNode*> ChildItem;
    void AddItem(const String& LablePath, const String& Tips, OnClick OnClickDelegate);
    void Draw();
};

class MenuBar : public UIWidget
{
public:
    MenuBarTreeRoot Root;

    explicit MenuBar(const String& Name): UIWidget(Name) {};
    explicit MenuBar() : UIWidget("MenuBar") {};

    virtual void Draw() override;

    // Lable path in A/B/C
    void AddItem(const String& LablePath, const String& Tips, OnClick OnClickDelegate);

	template<class T> requires IsMenuBarItem<T>
	void AddItem(String LablePath, String Tips)
	{
		auto TInstance = AddSubWidget<T>();
		AddItem(LablePath, Tips, [TInstance]() {
			TInstance->OnClick();
		});
	}
};
