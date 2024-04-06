#pragma once
#include "Game/Actor.h"

class Actor;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeActorOperator{
    public:
        static const char* getClassName(){ return "Actor";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(Actor);
            Actor* ret_instance= new Actor();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(Actor*)instance);
        }
        // base class
        static int getActorBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(Object,static_cast<Actor*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_Actor(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_Actor=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeActorOperator::getActorBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeActorOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeActorOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("Actor", class_function_tuple_Actor);
    }
namespace TypeWrappersRegister{
    void Actor()
    {
        TypeWrapperRegister_Actor();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

