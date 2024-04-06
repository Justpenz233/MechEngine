#pragma once
#include "Actors/CurveActor.h"

class CurveActor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeCurveActorOperator{
    public:
        static const char* getClassName(){ return "CurveActor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(CurveActor);
            CurveActor* ret_instance= new CurveActor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(CurveActor*)instance);
        }
        // base class
        static int getCurveActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Actor,static_cast<CurveActor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_CurveActor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_CurveActor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeCurveActorOperator::getCurveActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeCurveActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeCurveActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("CurveActor", class_function_tuple_CurveActor);
    }
namespace TypeWrappersRegister{
    void CurveActor()
    {
        TypeWrapperRegister_CurveActor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

