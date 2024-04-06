#pragma once
#include "Components/PointLightComponent.h"

class PointLightComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypePointLightComponentOperator{
    public:
        static const char* getClassName(){ return "PointLightComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(PointLightComponent);
            PointLightComponent* ret_instance= new PointLightComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(PointLightComponent*)instance);
        }
        // base class
        static int getPointLightComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(LightComponent,static_cast<PointLightComponent*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_PointLightComponent(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_PointLightComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypePointLightComponentOperator::getPointLightComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypePointLightComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypePointLightComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("PointLightComponent", class_function_tuple_PointLightComponent);
    }
namespace TypeWrappersRegister{
    void PointLightComponent()
    {
        TypeWrapperRegister_PointLightComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

