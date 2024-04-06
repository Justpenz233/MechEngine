#pragma once
#include "Components/ParametricMeshComponent.h"

class ParametricMeshComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeParametricMeshComponentOperator{
    public:
        static const char* getClassName(){ return "ParametricMeshComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(ParametricMeshComponent);
            ParametricMeshComponent* ret_instance= new ParametricMeshComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(ParametricMeshComponent*)instance);
        }
        // base class
        static int getParametricMeshComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(StaticMeshComponent,static_cast<ParametricMeshComponent*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_ParametricMeshComponent(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_ParametricMeshComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeParametricMeshComponentOperator::getParametricMeshComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeParametricMeshComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeParametricMeshComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("ParametricMeshComponent", class_function_tuple_ParametricMeshComponent);
    }
namespace TypeWrappersRegister{
    void ParametricMeshComponent()
    {
        TypeWrapperRegister_ParametricMeshComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

