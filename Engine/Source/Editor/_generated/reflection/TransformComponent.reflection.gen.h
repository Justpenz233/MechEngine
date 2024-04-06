#pragma once
#include "Components/TransformComponent.h"

class TransformComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeTransformComponentOperator{
    public:
        static const char* getClassName(){ return "TransformComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(TransformComponent);
            TransformComponent* ret_instance= new TransformComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(TransformComponent*)instance);
        }
        // base class
        static int getTransformComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(ActorComponent,static_cast<TransformComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_Transform(){ return "Transform";}
        static const char* getFieldTypeName_Transform(){ return "FTransform";}
        static void set_Transform(void* instance, void* field_value){ static_cast<TransformComponent*>(instance)->Transform = *static_cast<FTransform*>(field_value);}
        static void* get_Transform(void* instance){ return static_cast<void*>(&(static_cast<TransformComponent*>(instance)->Transform));}
        static bool isArray_Transform(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_TransformComponent(){
        FieldFunctionTuple* field_function_tuple_Transform=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::set_Transform,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::get_Transform,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::getFieldName_Transform,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::getFieldTypeName_Transform,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::isArray_Transform);
        REGISTER_FIELD_TO_MAP("TransformComponent", field_function_tuple_Transform);

        
        
        
        ClassFunctionTuple* class_function_tuple_TransformComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::getTransformComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeTransformComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("TransformComponent", class_function_tuple_TransformComponent);
    }
namespace TypeWrappersRegister{
    void TransformComponent()
    {
        TypeWrapperRegister_TransformComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

