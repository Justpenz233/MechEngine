#pragma once
#include "Components/StaticCurveComponent.h"

class StaticCurveComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeStaticCurveComponentOperator{
    public:
        static const char* getClassName(){ return "StaticCurveComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(StaticCurveComponent);
            StaticCurveComponent* ret_instance= new StaticCurveComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(StaticCurveComponent*)instance);
        }
        // base class
        static int getStaticCurveComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(StaticMeshComponent,static_cast<StaticCurveComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_Radius(){ return "Radius";}
        static const char* getFieldTypeName_Radius(){ return "double";}
        static void set_Radius(void* instance, void* field_value){ static_cast<StaticCurveComponent*>(instance)->Radius = *static_cast<double*>(field_value);}
        static void* get_Radius(void* instance){ return static_cast<void*>(&(static_cast<StaticCurveComponent*>(instance)->Radius));}
        static bool isArray_Radius(){ return false; }
        static const char* getFieldName_SampleNum(){ return "SampleNum";}
        static const char* getFieldTypeName_SampleNum(){ return "int";}
        static void set_SampleNum(void* instance, void* field_value){ static_cast<StaticCurveComponent*>(instance)->SampleNum = *static_cast<int*>(field_value);}
        static void* get_SampleNum(void* instance){ return static_cast<void*>(&(static_cast<StaticCurveComponent*>(instance)->SampleNum));}
        static bool isArray_SampleNum(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_StaticCurveComponent(){
        FieldFunctionTuple* field_function_tuple_Radius=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::set_Radius,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::get_Radius,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getFieldName_Radius,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getFieldTypeName_Radius,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::isArray_Radius);
        REGISTER_FIELD_TO_MAP("StaticCurveComponent", field_function_tuple_Radius);
        FieldFunctionTuple* field_function_tuple_SampleNum=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::set_SampleNum,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::get_SampleNum,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getFieldName_SampleNum,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getFieldTypeName_SampleNum,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::isArray_SampleNum);
        REGISTER_FIELD_TO_MAP("StaticCurveComponent", field_function_tuple_SampleNum);

        
        
        
        ClassFunctionTuple* class_function_tuple_StaticCurveComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::getStaticCurveComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeStaticCurveComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("StaticCurveComponent", class_function_tuple_StaticCurveComponent);
    }
namespace TypeWrappersRegister{
    void StaticCurveComponent()
    {
        TypeWrapperRegister_StaticCurveComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

