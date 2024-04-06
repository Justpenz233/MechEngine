#pragma once
#include "../../../DevelopableMechanism/Source/SpacialMechanism/SurfaceJoint.h"

class SurfaceJointActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeSurfaceJointActorOperator{
    public:
        static const char* getClassName(){ return "SurfaceJointActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(SurfaceJointActor);
            SurfaceJointActor* ret_instance= new SurfaceJointActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(SurfaceJointActor*)instance);
        }
        // base class
        static int getSurfaceJointActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<SurfaceJointActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_SurfaceJointActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_SurfaceJointActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointActorOperator::getSurfaceJointActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeSurfaceJointActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeSurfaceJointActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("SurfaceJointActor", class_function_tuple_SurfaceJointActor);
    }
namespace TypeWrappersRegister{
    void SurfaceJoint()
    {
        TypeWrapperRegister_SurfaceJointActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

