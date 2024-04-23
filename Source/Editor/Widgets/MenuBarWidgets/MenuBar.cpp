#include "MenuBar.h"
#include "Log/Log.h"
#include "Containers/FString.h"
#include "imgui.h"

MenuBarTreeNode::~MenuBarTreeNode()
{
    for(auto Child : ChildItem)
        delete Child;
    ChildItem.clear();
}

MenuBarTreeRoot::~MenuBarTreeRoot()
{
    for(auto Child : ChildItem)
        delete Child;
    ChildItem.clear();
}

MenuBarTreeNode::MenuBarTreeNode(TArray<String> InLabel, String InTips, OnClick InOnClickDelegate)
{
    Label = InLabel[0];
    if(InLabel.size() == 1)
    {
        Tips = InTips;
        OnClickDelegate = InOnClickDelegate;
    }
    else
    {
        InLabel.erase(InLabel.begin());
        ChildItem.push_back(new MenuBarTreeNode(InLabel, InTips, InOnClickDelegate));
    }
}

void MenuBarTreeNode::Draw()
{
    if(IsItem())
    {
        if(ImGui::MenuItem(Label.c_str(), Tips.c_str()))
        {
            OnClickDelegate.ExecuteIfBound();
        }
    }
    else {
        if(ImGui::BeginMenu(Label.c_str()))
        {
            for(auto Child : ChildItem)
                Child->Draw();
            ImGui::EndMenu();
        }
    }
}

void MenuBarTreeNode::AddItem(TArray<String> LablePath, String Tips, OnClick OnClickDelegate)
{
    for(auto Child : ChildItem)
    {
        if(Child->Label == LablePath[0])
        {
            ASSERTMSG(LablePath.size() > 1, "Lable already exists");
            LablePath.erase(LablePath.begin());
            Child->AddItem(LablePath, Tips, OnClickDelegate);
            return;
        }
    }
    ChildItem.push_back( new MenuBarTreeNode(LablePath, Tips, OnClickDelegate));
}

void MenuBarTreeRoot::AddItem(const String& LablePath, const String& Tips, OnClick OnClickDelegate)
{
    auto Labels = FString::SpiltBy(LablePath, '/');
    ASSERTMSG(Labels.size() > 0, "Label cannot be empty, please follow A/B/...");

    for(auto Child : ChildItem)
    {
        if(Child->Label == Labels[0])
        {
            ASSERTMSG(Labels.size() > 1, "Lable {} already exists", LablePath);
            Labels.erase(Labels.begin());
            Child->AddItem(Labels, Tips, OnClickDelegate);
            return;
        }
    }
    ChildItem.push_back( new MenuBarTreeNode(Labels, Tips, OnClickDelegate));
}

void MenuBarTreeRoot::Draw()
{
    if(ImGui::BeginMainMenuBar())
    {
        for(auto Child : ChildItem)
            Child->Draw();
        
        ImGui::EndMainMenuBar();
    }
}

void MenuBar::Draw()
{
    Root.Draw();
}

void MenuBar::AddItem(const String& LablePath, const String& Tips, OnClick OnClickDelegate)
{
    Root.AddItem(LablePath, Tips, OnClickDelegate);
}

