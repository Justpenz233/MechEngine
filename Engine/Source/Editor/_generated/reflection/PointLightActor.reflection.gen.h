#pragma once
#include "Actors/PointLightActor.h"

class PointLightActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypePointLightActorOperator{
    public:
        static const char* getClassName(){ return "PointLightActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(PointLightActor);
            PointLightActor* ret_instance= new PointLightActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(PointLightActor*)instance);
        }
        // base class
        static int getPointLightActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<PointLightActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_PointLightActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_PointLightActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypePointLightActorOperator::getPointLightActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypePointLightActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypePointLightActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("PointLightActor", class_function_tuple_PointLightActor);
    }
namespace TypeWrappersRegister{
    void PointLightActor()
    {
        TypeWrapperRegister_PointLightActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

