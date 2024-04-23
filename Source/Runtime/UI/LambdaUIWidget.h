#pragma once
#include "UIWidget.h"


class LambdaUIWidget: public UIWidget
{
protected:
    std::function<void()> Func;

public:
    LambdaUIWidget(std::function<void()> _Func) : Func(std::move(_Func)), UIWidget("LambdaWidget") 
    {}

    virtual void Draw() override
    {
        if(Visible) Func();
    }
};