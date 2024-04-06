#pragma once
#include "Core/Object/Object.h"

class Object;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeObjectOperator{
    public:
        static const char* getClassName(){ return "Object";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(Object);
            Object* ret_instance= new Object();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(Object*)instance);
        }
        // base class
        static int getObjectBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* getFieldName_ObjectName(){ return "ObjectName";}
        static const char* getFieldTypeName_ObjectName(){ return "String";}
        static void set_ObjectName(void* instance, void* field_value){ static_cast<Object*>(instance)->ObjectName = *static_cast<String*>(field_value);}
        static void* get_ObjectName(void* instance){ return static_cast<void*>(&(static_cast<Object*>(instance)->ObjectName));}
        static bool isArray_ObjectName(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_Object(){
        FieldFunctionTuple* field_function_tuple_ObjectName=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeObjectOperator::set_ObjectName,
            &TypeFieldReflectionOparator::TypeObjectOperator::get_ObjectName,
            &TypeFieldReflectionOparator::TypeObjectOperator::getClassName,
            &TypeFieldReflectionOparator::TypeObjectOperator::getFieldName_ObjectName,
            &TypeFieldReflectionOparator::TypeObjectOperator::getFieldTypeName_ObjectName,
            &TypeFieldReflectionOparator::TypeObjectOperator::isArray_ObjectName);
        REGISTER_FIELD_TO_MAP("Object", field_function_tuple_ObjectName);

        
        
        
        ClassFunctionTuple* class_function_tuple_Object=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeObjectOperator::getObjectBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeObjectOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeObjectOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("Object", class_function_tuple_Object);
    }
namespace TypeWrappersRegister{
    void Object()
    {
        TypeWrapperRegister_Object();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

