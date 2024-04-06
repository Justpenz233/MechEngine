#pragma once
#include "Components/RenderingComponent.h"

class RenderingComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeRenderingComponentOperator{
    public:
        static const char* getClassName(){ return "RenderingComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(RenderingComponent);
            RenderingComponent* ret_instance= new RenderingComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(RenderingComponent*)instance);
        }
        // base class
        static int getRenderingComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(ActorComponent,static_cast<RenderingComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_bCastShadow(){ return "bCastShadow";}
        static const char* getFieldTypeName_bCastShadow(){ return "bool";}
        static void set_bCastShadow(void* instance, void* field_value){ static_cast<RenderingComponent*>(instance)->bCastShadow = *static_cast<bool*>(field_value);}
        static void* get_bCastShadow(void* instance){ return static_cast<void*>(&(static_cast<RenderingComponent*>(instance)->bCastShadow));}
        static bool isArray_bCastShadow(){ return false; }
        static const char* getFieldName_bVisible(){ return "bVisible";}
        static const char* getFieldTypeName_bVisible(){ return "bool";}
        static void set_bVisible(void* instance, void* field_value){ static_cast<RenderingComponent*>(instance)->bVisible = *static_cast<bool*>(field_value);}
        static void* get_bVisible(void* instance){ return static_cast<void*>(&(static_cast<RenderingComponent*>(instance)->bVisible));}
        static bool isArray_bVisible(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_RenderingComponent(){
        FieldFunctionTuple* field_function_tuple_bCastShadow=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::set_bCastShadow,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::get_bCastShadow,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getFieldName_bCastShadow,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getFieldTypeName_bCastShadow,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::isArray_bCastShadow);
        REGISTER_FIELD_TO_MAP("RenderingComponent", field_function_tuple_bCastShadow);
        FieldFunctionTuple* field_function_tuple_bVisible=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::set_bVisible,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::get_bVisible,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getFieldName_bVisible,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getFieldTypeName_bVisible,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::isArray_bVisible);
        REGISTER_FIELD_TO_MAP("RenderingComponent", field_function_tuple_bVisible);

        
        
        
        ClassFunctionTuple* class_function_tuple_RenderingComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::getRenderingComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeRenderingComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("RenderingComponent", class_function_tuple_RenderingComponent);
    }
namespace TypeWrappersRegister{
    void RenderingComponent()
    {
        TypeWrapperRegister_RenderingComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

