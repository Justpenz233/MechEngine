#pragma once
#include "Game/StaticMeshActor.h"

class StaticMeshActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeStaticMeshActorOperator{
    public:
        static const char* getClassName(){ return "StaticMeshActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(StaticMeshActor);
            StaticMeshActor* ret_instance= new StaticMeshActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(StaticMeshActor*)instance);
        }
        // base class
        static int getStaticMeshActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<StaticMeshActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_StaticMeshActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_StaticMeshActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticMeshActorOperator::getStaticMeshActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeStaticMeshActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeStaticMeshActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("StaticMeshActor", class_function_tuple_StaticMeshActor);
    }
namespace TypeWrappersRegister{
    void StaticMeshActor()
    {
        TypeWrapperRegister_StaticMeshActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

