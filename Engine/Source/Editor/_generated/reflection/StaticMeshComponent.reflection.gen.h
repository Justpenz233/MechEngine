#pragma once
#include "Components/StaticMeshComponent.h"

class StaticMeshComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeStaticMeshComponentOperator{
    public:
        static const char* getClassName(){ return "StaticMeshComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(StaticMeshComponent);
            StaticMeshComponent* ret_instance= new StaticMeshComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(StaticMeshComponent*)instance);
        }
        // base class
        static int getStaticMeshComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(RenderingComponent,static_cast<StaticMeshComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_Color(){ return "Color";}
        static const char* getFieldTypeName_Color(){ return "FColor";}
        static void set_Color(void* instance, void* field_value){ static_cast<StaticMeshComponent*>(instance)->Color = *static_cast<FColor*>(field_value);}
        static void* get_Color(void* instance){ return static_cast<void*>(&(static_cast<StaticMeshComponent*>(instance)->Color));}
        static bool isArray_Color(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_StaticMeshComponent(){
        FieldFunctionTuple* field_function_tuple_Color=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::set_Color,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::get_Color,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::getFieldName_Color,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::getFieldTypeName_Color,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::isArray_Color);
        REGISTER_FIELD_TO_MAP("StaticMeshComponent", field_function_tuple_Color);

        
        
        
        ClassFunctionTuple* class_function_tuple_StaticMeshComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::getStaticMeshComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeStaticMeshComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("StaticMeshComponent", class_function_tuple_StaticMeshComponent);
    }
namespace TypeWrappersRegister{
    void StaticMeshComponent()
    {
        TypeWrapperRegister_StaticMeshComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

