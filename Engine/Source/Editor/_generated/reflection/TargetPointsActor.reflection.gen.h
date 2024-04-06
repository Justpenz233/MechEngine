#pragma once
#include "../../../DevelopableMechanism/Source/MeshFitting/TargetPointsActor.h"

class TargetPointsActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeTargetPointsActorOperator{
    public:
        static const char* getClassName(){ return "TargetPointsActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(TargetPointsActor);
            TargetPointsActor* ret_instance= new TargetPointsActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(TargetPointsActor*)instance);
        }
        // base class
        static int getTargetPointsActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(StaticMeshActor,static_cast<TargetPointsActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_TargetPointsActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_TargetPointsActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeTargetPointsActorOperator::getTargetPointsActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeTargetPointsActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeTargetPointsActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("TargetPointsActor", class_function_tuple_TargetPointsActor);
    }
namespace TypeWrappersRegister{
    void TargetPointsActor()
    {
        TypeWrapperRegister_TargetPointsActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

