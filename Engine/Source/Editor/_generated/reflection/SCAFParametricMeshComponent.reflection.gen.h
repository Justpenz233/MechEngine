#pragma once
#include "Components/SCAFParametricMeshComponent.h"

class SCAFParametricMeshComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeSCAFParametricMeshComponentOperator{
    public:
        static const char* getClassName(){ return "SCAFParametricMeshComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(SCAFParametricMeshComponent);
            SCAFParametricMeshComponent* ret_instance= new SCAFParametricMeshComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(SCAFParametricMeshComponent*)instance);
        }
        // base class
        static int getSCAFParametricMeshComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(ParametricMeshComponent,static_cast<SCAFParametricMeshComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_bShowUV(){ return "bShowUV";}
        static const char* getFieldTypeName_bShowUV(){ return "bool";}
        static void set_bShowUV(void* instance, void* field_value){ static_cast<SCAFParametricMeshComponent*>(instance)->bShowUV = *static_cast<bool*>(field_value);}
        static void* get_bShowUV(void* instance){ return static_cast<void*>(&(static_cast<SCAFParametricMeshComponent*>(instance)->bShowUV));}
        static bool isArray_bShowUV(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_SCAFParametricMeshComponent(){
        FieldFunctionTuple* field_function_tuple_bShowUV=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::set_bShowUV,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::get_bShowUV,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::getFieldName_bShowUV,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::getFieldTypeName_bShowUV,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::isArray_bShowUV);
        REGISTER_FIELD_TO_MAP("SCAFParametricMeshComponent", field_function_tuple_bShowUV);

        
        
        
        ClassFunctionTuple* class_function_tuple_SCAFParametricMeshComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::getSCAFParametricMeshComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeSCAFParametricMeshComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("SCAFParametricMeshComponent", class_function_tuple_SCAFParametricMeshComponent);
    }
namespace TypeWrappersRegister{
    void SCAFParametricMeshComponent()
    {
        TypeWrapperRegister_SCAFParametricMeshComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

