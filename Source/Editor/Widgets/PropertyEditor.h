//
// Created by MarvelLi on 2024/1/18.
//
#pragma once
#include "Reflection/reflection/reflection.h"

namespace MechEngine
{
    namespace Widgets
    {
        using PropertyWidget = std::function<void(void* instance, Reflection::FieldAccessor& Field)>;
    	using EnumWidget = std::function<void(Reflection::EnumAccessor& Field)>;
    	using ArrayWidget = std::function<void(Reflection::ArrayAccessor& Field)>;

        PropertyWidget CreateStringEditWidget();

        PropertyWidget CreateIntEditWidget();

    	PropertyWidget CreateBoolEditWidget();

    	PropertyWidget CreateFloatEditWidget();

    	PropertyWidget CreateDoubleEditWidget();

        PropertyWidget CreateFVectorWidget();

    	PropertyWidget CreateFColorWidget();

    	PropertyWidget CreateFTransformWidget();

    	EnumWidget CreateEnumComboWidget();

        FORCEINLINE PropertyWidget PropertyEditDraw(::Reflection::FieldAccessor& Field)
        {
            std::string TypeName = Field.GetPureTypeName();
            if(TypeName == "string") return CreateStringEditWidget();
            if(TypeName == "int") return CreateIntEditWidget();
            if(TypeName == "FVector") return CreateFVectorWidget();
        	if(TypeName == "bool") return CreateBoolEditWidget();
        	if(TypeName == "float") return CreateFloatEditWidget();
        	if(TypeName == "double") return CreateDoubleEditWidget();
        	if(TypeName == "FColor" || TypeName == "FLinearColor") return CreateFColorWidget();
        	if(TypeName == "FTransform") return CreateFTransformWidget();
            return nullptr;
        }

    	FORCEINLINE EnumWidget PropertyEditDraw(::Reflection::EnumAccessor& Field)
        {
	        return CreateEnumComboWidget();
        }


    }
}

