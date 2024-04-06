#pragma once
#include "Components/ParametricSurfaceComponent.h"

class ParametricSurfaceComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeParametricSurfaceComponentOperator{
    public:
        static const char* getClassName(){ return "ParametricSurfaceComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(ParametricSurfaceComponent);
            ParametricSurfaceComponent* ret_instance= new ParametricSurfaceComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(ParametricSurfaceComponent*)instance);
        }
        // base class
        static int getParametricSurfaceComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(ParametricMeshComponent,static_cast<ParametricSurfaceComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_Thickness(){ return "Thickness";}
        static const char* getFieldTypeName_Thickness(){ return "double";}
        static void set_Thickness(void* instance, void* field_value){ static_cast<ParametricSurfaceComponent*>(instance)->Thickness = *static_cast<double*>(field_value);}
        static void* get_Thickness(void* instance){ return static_cast<void*>(&(static_cast<ParametricSurfaceComponent*>(instance)->Thickness));}
        static bool isArray_Thickness(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_ParametricSurfaceComponent(){
        FieldFunctionTuple* field_function_tuple_Thickness=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::set_Thickness,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::get_Thickness,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::getFieldName_Thickness,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::getFieldTypeName_Thickness,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::isArray_Thickness);
        REGISTER_FIELD_TO_MAP("ParametricSurfaceComponent", field_function_tuple_Thickness);

        
        
        
        ClassFunctionTuple* class_function_tuple_ParametricSurfaceComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::getParametricSurfaceComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeParametricSurfaceComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("ParametricSurfaceComponent", class_function_tuple_ParametricSurfaceComponent);
    }
namespace TypeWrappersRegister{
    void ParametricSurfaceComponent()
    {
        TypeWrapperRegister_ParametricSurfaceComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

