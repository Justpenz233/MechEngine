//
// Created by MarvelLi on 2024/1/18.
//
#include "PropertyEditor.h"
#include "CoreMinimal.h"
#include "PrettifyName.h"
#include "Math/FTransform.h"
#include "imgui.h"
#include "Math/LinearAlgebra.h"
#include "Math/Math.h"
#include <IconsFontAwesome6.h>

#include "imgui_toggle/imgui_toggle.h"

namespace MechEngine
{
    namespace Widgets
    {
    	void Float3CopyFVector(float* Value, const FVector& Vec)
    	{ for (int i = 0; i < 3; i++) Value[i] = static_cast<float>(Vec[i]); }
    	void Float3ToDeg(float* Value)
    	{ for (int i = 0; i < 3; i++) Value[i] = Math::RadToDeg(Value[i]); }
    	void Float3ToRad(float* Value)
    	{ for (int i = 0; i < 3; i++) Value[i] = Math::DegToRad(Value[i]); }

        void ShowLableLeft(const std::string& Lable)
        {
            ImGui::Text("%s", Lable.c_str());
            ImGui::SameLine();
        }
        PropertyWidget CreateStringEditWidget()
        {
            return [](void* instance, Reflection::FieldAccessor& Field)
            {
                static char Buffer[256];
                strcpy(Buffer, Field.get<std::string>(instance).c_str());
                std::string Lable = Field.getFieldName();
            	Lable = UI::PretifyUIName(Lable);
                ShowLableLeft(Lable);
                ImGui::PushID(Lable.c_str());
            	ImGui::InputText("", Buffer, 256);
                if(ImGui::IsItemDeactivatedAfterEdit())
                    Field.set(instance, Buffer);
                ImGui::PopID();
            };
        }

        PropertyWidget CreateIntEditWidget()
		{
			return [](void* instance, Reflection::FieldAccessor& Field) {
				auto Value = static_cast<int*>(Field.get(instance));
				std::string	 Lable = Field.getFieldName();
				Lable = UI::PretifyUIName(Lable);
				ShowLableLeft(Lable);
				ImGui::PushID(Lable.c_str());
				if(auto SlideTag = Field.GetPropertyTag<Slide_>())
				{
					ImGui::SliderInt("", Value, SlideTag->GetMin(), SlideTag->GetMax());
				} else if(auto StepTag = Field.GetPropertyTag<Step_>())
				{
					ImGui::InputInt("", Value, StepTag->GetStep(), StepTag->GetStep() * 10);
				}else ImGui::InputInt("", Value);
				if(ImGui::IsItemDeactivatedAfterEdit())
					Field.set(instance, Value);
				ImGui::PopID();
			};
		}
		PropertyWidget CreateBoolEditWidget()
		{
			return [](void* instance, Reflection::FieldAccessor& Field) {
				static bool Value;
				Value = Field.get<bool>(instance);
				std::string Lable = Field.getFieldName();
				Lable = UI::PretifyUIName(Lable);
				ShowLableLeft(Lable);
				ImGui::PushID(Lable.c_str());
				constexpr ImVec4 gray_dim(0.45f, 0.45f, 0.45f, 1.0f);
				constexpr ImVec4 gray(0.65f, 0.65f, 0.65f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, gray_dim);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, gray);
				if (ImGui::Toggle("", &Value, ImGuiToggleFlags_Animated))
					Field.set(instance, &Value);
				ImGui::PopStyleColor(2);
				ImGui::PopID();
			};
		}

		PropertyWidget CreateFloatEditWidget()
		{
			return [](void* instance, Reflection::FieldAccessor& Field) {
				auto Value = static_cast<float*>(Field.get(instance));
				std::string	 Lable = Field.getFieldName();
				Lable = UI::PretifyUIName(Lable);
				ShowLableLeft(Lable);
				ImGui::PushID(Lable.c_str());
				if(auto SlideTag = Field.GetPropertyTag<Slide_>())
				{
					ImGui::SliderFloat("", Value, SlideTag->GetMin(), SlideTag->GetMax());
				} else   // Have critical bugs, should make Imgui ClassOriented
				if(auto StepTag = Field.GetPropertyTag<Step_>())
				{
					ImGui::InputFloat("", Value, StepTag->GetStep(), StepTag->GetStep() * 10);
					*Value = std::clamp(*Value, StepTag->GetMin(), StepTag->GetMax());
				}
				else
					ImGui::InputFloat("", Value);

				if(ImGui::IsItemDeactivatedAfterEdit())
					Field.set(instance, Value);
				ImGui::PopID();
			};
		}
		PropertyWidget CreateDoubleEditWidget()
		{
        	return [](void* instance, Reflection::FieldAccessor& Field) {
        		auto Value = static_cast<double*>(Field.get(instance));
        		auto fValue = static_cast<float>(*Value);
        		std::string	 Lable = Field.getFieldName();
        		Lable = UI::PretifyUIName(Lable);
        		ShowLableLeft(Lable);
        		ImGui::PushID(Lable.c_str());
        		if(auto SlideTag = Field.GetPropertyTag<Slide_>())
        		{
        			if(ImGui::SliderFloat("", &fValue, SlideTag->GetMin(), SlideTag->GetMax()))
        				*Value = fValue;
        		} else if(auto StepTag = Field.GetPropertyTag<Step_>())
        		{
        			if(ImGui::InputFloat("", &fValue, StepTag->GetStep(), StepTag->GetStep() * 10))
        				*Value = std::clamp(fValue, StepTag->GetMin(), StepTag->GetMax());
        		}else if(ImGui::InputFloat("", &fValue)) *Value = fValue;
        		if(ImGui::IsItemDeactivatedAfterEdit())
        			Field.set(instance, Value);
        		ImGui::PopID();
        	};
		}

		PropertyWidget CreateFVectorWidget()
		{
			return [](void* instance, Reflection::FieldAccessor& Field) {
				static float Value[3] = { 0, 0, 0 };
				FVector		 CurrentVec = Field.get<FVector>(instance);
				Value[0] = CurrentVec.x();
				Value[1] = CurrentVec.y();
				Value[2] = CurrentVec.z();
				std::string Lable = Field.getFieldName();
				Lable = UI::PretifyUIName(Lable);
				ShowLableLeft(Lable);
				ImGui::PushID(Lable.c_str());
				ImGui::InputFloat3("", Value);
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					FVector Vec(Value[0], Value[1], Value[2]);
					Field.set(instance, &Vec);
				}
				ImGui::PopID();
			};
		}
    	PropertyWidget CreateFVector2Widget()
    	{
    		return [](void* instance, Reflection::FieldAccessor& Field) {
    			static float Value[2] = { 0, 0 };
    			FVector2	CurrentVec = Field.get<FVector2>(instance);
    			Value[0] = CurrentVec.x();
    			Value[1] = CurrentVec.y();
    			std::string Lable = Field.getFieldName();
    			Lable = UI::PretifyUIName(Lable);
    			ShowLableLeft(Lable);
    			ImGui::PushID(Lable.c_str());
    			ImGui::InputFloat2("", Value);
    			if (ImGui::IsItemDeactivatedAfterEdit())
    			{
    				FVector2 Vec(Value[0], Value[1]);
    				Field.set(instance, &Vec);
    			}
    			ImGui::PopID();
    		};
    	}
		PropertyWidget CreateFColorWidget()
		{
			return [](void* instance, Reflection::FieldAccessor& Field) {
				static float Value[3] = { 0, 0, 0 };
				FColor CurrentColor = Field.get<FColor>(instance);
				Value[0] = CurrentColor.x();
				Value[1] = CurrentColor.y();
				Value[2] = CurrentColor.z();
				std::string Lable = Field.getFieldName();
				Lable = UI::PretifyUIName(Lable);
				ShowLableLeft(ICON_FA_PALETTE " " + Lable);
				ImGui::PushID((Lable).c_str());
				if (ImGui::ColorEdit3("", Value))
				{
					FColor Color(Value[0], Value[1], Value[2]);
					Field.set(instance, &Color);
				}
				ImGui::PopID();
			};
		}
		PropertyWidget CreateFTransformWidget()
		{
        	return [](void* instance, Reflection::FieldAccessor& Field) {
        		static FTransform Value;
        		static float Translation[3];
        		static float Rotation[3];
        		static float Scale[3];

        		Value = Field.get<FTransform>(instance);
        		Float3CopyFVector(Translation, Value.GetTranslation());
        		Float3CopyFVector(Rotation, Value.GetRotationEuler());
        		Float3ToDeg(Rotation);
        		Float3CopyFVector(Scale, Value.GetScale());
        		std::string Lable = Field.getFieldName();
        		Lable = UI::PretifyUIName(Lable);
        		ImGui::PushID(Lable.c_str());
        		ImGui::InputFloat3("Translation", Translation);
        		if (ImGui::IsItemDeactivatedAfterEdit())
        		{
        			Value.SetTranslation(FVector(Translation[0], Translation[1], Translation[2]));
					Field.set(instance, &Value);
        		}
        		ImGui::InputFloat3("Rotation", Rotation);
        		if (ImGui::IsItemDeactivatedAfterEdit())
        		{
        			TArray<double> RotationRad(3);
        			for (int i = 0; i < 3; i++) RotationRad[i] = static_cast<double>(Math::DegToRad(Rotation[i]));
        			Value.SetRotation(MMath::QuaternionFromEulerXYZ(FVector(RotationRad[0], RotationRad[1], RotationRad[2])));
        			Field.set(instance, &Value);
        		}
        		ImGui::InputFloat3("Scale", Scale);
        		if (ImGui::IsItemDeactivatedAfterEdit())
        		{
        			Value.SetScale(FVector(Scale[0], Scale[1], Scale[2]));
        			Field.set(instance, &Value);
        		}
        		ImGui::PopID();
        	};
		}

		EnumWidget CreateEnumComboWidget()
    	{
    		return [](Reflection::EnumAccessor& Field) {
    			auto Current = Field.GetEnumValue();
    			auto Items = Field.GetEnumStringArray();

    			auto EnumClassName = Field.GetEnumTypeName();
    			String Label = UI::PretifyUIName(EnumClassName);
    			ImGui::PushID(Field.GetFiledName());
    			ShowLableLeft(Label);
				if(ImGui::BeginCombo("", Current.c_str()))
				{
					for (const auto & Item : Items)
					{
						bool is_selected = (Current == Item);
						auto ItemName = UI::PretifyUIName(Item);
						if (ImGui::Selectable(ItemName.c_str(), is_selected))
						{
							if(Current != Item)
							{
								Current = Item;
								Field.SetEnumValue(Current);
							}
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
    			ImGui::PopID();
    		};
    	}
    } // namespace Widgets
}
