#pragma once
#include "Actors/ParametricMeshActor.h"

class ParametricMeshActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeParametricMeshActorOperator{
    public:
        static const char* getClassName(){ return "ParametricMeshActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(ParametricMeshActor);
            ParametricMeshActor* ret_instance= new ParametricMeshActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(ParametricMeshActor*)instance);
        }
        // base class
        static int getParametricMeshActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<ParametricMeshActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_ParametricMeshActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_ParametricMeshActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeParametricMeshActorOperator::getParametricMeshActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeParametricMeshActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeParametricMeshActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("ParametricMeshActor", class_function_tuple_ParametricMeshActor);
    }
namespace TypeWrappersRegister{
    void ParametricMeshActor()
    {
        TypeWrapperRegister_ParametricMeshActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

