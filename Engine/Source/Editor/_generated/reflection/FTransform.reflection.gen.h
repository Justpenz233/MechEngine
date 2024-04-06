#pragma once
#include "Core/Math/FTransform.h"

class FTransform;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeFTransformOperator{
    public:
        static const char* getClassName(){ return "FTransform";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(FTransform);
            FTransform* ret_instance= new FTransform();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(FTransform*)instance);
        }
        // base class
        static int getFTransformBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 0;
            
            return count;
        }
        // fields
        static const char* getFieldName_Translation(){ return "Translation";}
        static const char* getFieldTypeName_Translation(){ return "FVector";}
        static void set_Translation(void* instance, void* field_value){ static_cast<FTransform*>(instance)->Translation = *static_cast<FVector*>(field_value);}
        static void* get_Translation(void* instance){ return static_cast<void*>(&(static_cast<FTransform*>(instance)->Translation));}
        static bool isArray_Translation(){ return false; }
        static const char* getFieldName_Rotation(){ return "Rotation";}
        static const char* getFieldTypeName_Rotation(){ return "FQuat";}
        static void set_Rotation(void* instance, void* field_value){ static_cast<FTransform*>(instance)->Rotation = *static_cast<FQuat*>(field_value);}
        static void* get_Rotation(void* instance){ return static_cast<void*>(&(static_cast<FTransform*>(instance)->Rotation));}
        static bool isArray_Rotation(){ return false; }
        static const char* getFieldName_Scale(){ return "Scale";}
        static const char* getFieldTypeName_Scale(){ return "FVector";}
        static void set_Scale(void* instance, void* field_value){ static_cast<FTransform*>(instance)->Scale = *static_cast<FVector*>(field_value);}
        static void* get_Scale(void* instance){ return static_cast<void*>(&(static_cast<FTransform*>(instance)->Scale));}
        static bool isArray_Scale(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_FTransform(){
        FieldFunctionTuple* field_function_tuple_Translation=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeFTransformOperator::set_Translation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::get_Translation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getClassName,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldName_Translation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldTypeName_Translation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::isArray_Translation);
        REGISTER_FIELD_TO_MAP("FTransform", field_function_tuple_Translation);
        FieldFunctionTuple* field_function_tuple_Rotation=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeFTransformOperator::set_Rotation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::get_Rotation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getClassName,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldName_Rotation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldTypeName_Rotation,
            &TypeFieldReflectionOparator::TypeFTransformOperator::isArray_Rotation);
        REGISTER_FIELD_TO_MAP("FTransform", field_function_tuple_Rotation);
        FieldFunctionTuple* field_function_tuple_Scale=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeFTransformOperator::set_Scale,
            &TypeFieldReflectionOparator::TypeFTransformOperator::get_Scale,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getClassName,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldName_Scale,
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFieldTypeName_Scale,
            &TypeFieldReflectionOparator::TypeFTransformOperator::isArray_Scale);
        REGISTER_FIELD_TO_MAP("FTransform", field_function_tuple_Scale);

        
        
        
        ClassFunctionTuple* class_function_tuple_FTransform=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeFTransformOperator::getFTransformBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeFTransformOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeFTransformOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("FTransform", class_function_tuple_FTransform);
    }
namespace TypeWrappersRegister{
    void FTransform()
    {
        TypeWrapperRegister_FTransform();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

