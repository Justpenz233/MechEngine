#pragma once
#include "Game/CameraActor.h"

class CameraActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeCameraActorOperator{
    public:
        static const char* getClassName(){ return "CameraActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(CameraActor);
            CameraActor* ret_instance= new CameraActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(CameraActor*)instance);
        }
        // base class
        static int getCameraActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<CameraActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_CameraActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_CameraActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeCameraActorOperator::getCameraActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeCameraActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeCameraActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("CameraActor", class_function_tuple_CameraActor);
    }
namespace TypeWrappersRegister{
    void CameraActor()
    {
        TypeWrapperRegister_CameraActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

