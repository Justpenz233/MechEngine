#pragma once
#include "Components/LightComponent.h"

class LightComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeLightComponentOperator{
    public:
        static const char* getClassName(){ return "LightComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(LightComponent);
            LightComponent* ret_instance= new LightComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(LightComponent*)instance);
        }
        // base class
        static int getLightComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(RenderingComponent,static_cast<LightComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_Intensity(){ return "Intensity";}
        static const char* getFieldTypeName_Intensity(){ return "float";}
        static void set_Intensity(void* instance, void* field_value){ static_cast<LightComponent*>(instance)->Intensity = *static_cast<float*>(field_value);}
        static void* get_Intensity(void* instance){ return static_cast<void*>(&(static_cast<LightComponent*>(instance)->Intensity));}
        static bool isArray_Intensity(){ return false; }
        static const char* getFieldName_LightColor(){ return "LightColor";}
        static const char* getFieldTypeName_LightColor(){ return "FColor";}
        static void set_LightColor(void* instance, void* field_value){ static_cast<LightComponent*>(instance)->LightColor = *static_cast<FColor*>(field_value);}
        static void* get_LightColor(void* instance){ return static_cast<void*>(&(static_cast<LightComponent*>(instance)->LightColor));}
        static bool isArray_LightColor(){ return false; }
        static const char* getFieldName_SamplesPerPixel(){ return "SamplesPerPixel";}
        static const char* getFieldTypeName_SamplesPerPixel(){ return "int";}
        static void set_SamplesPerPixel(void* instance, void* field_value){ static_cast<LightComponent*>(instance)->SamplesPerPixel = *static_cast<int*>(field_value);}
        static void* get_SamplesPerPixel(void* instance){ return static_cast<void*>(&(static_cast<LightComponent*>(instance)->SamplesPerPixel));}
        static bool isArray_SamplesPerPixel(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_LightComponent(){
        FieldFunctionTuple* field_function_tuple_Intensity=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeLightComponentOperator::set_Intensity,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::get_Intensity,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldName_Intensity,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldTypeName_Intensity,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::isArray_Intensity);
        REGISTER_FIELD_TO_MAP("LightComponent", field_function_tuple_Intensity);
        FieldFunctionTuple* field_function_tuple_LightColor=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeLightComponentOperator::set_LightColor,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::get_LightColor,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldName_LightColor,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldTypeName_LightColor,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::isArray_LightColor);
        REGISTER_FIELD_TO_MAP("LightComponent", field_function_tuple_LightColor);
        FieldFunctionTuple* field_function_tuple_SamplesPerPixel=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeLightComponentOperator::set_SamplesPerPixel,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::get_SamplesPerPixel,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldName_SamplesPerPixel,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getFieldTypeName_SamplesPerPixel,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::isArray_SamplesPerPixel);
        REGISTER_FIELD_TO_MAP("LightComponent", field_function_tuple_SamplesPerPixel);

        
        
        
        ClassFunctionTuple* class_function_tuple_LightComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeLightComponentOperator::getLightComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeLightComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("LightComponent", class_function_tuple_LightComponent);
    }
namespace TypeWrappersRegister{
    void LightComponent()
    {
        TypeWrapperRegister_LightComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

