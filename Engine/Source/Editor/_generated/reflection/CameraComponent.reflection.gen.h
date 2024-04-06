#pragma once
#include "Components/CameraComponent.h"

class CameraComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeCameraComponentOperator{
    public:
        static const char* getClassName(){ return "CameraComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(CameraComponent);
            CameraComponent* ret_instance= new CameraComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(CameraComponent*)instance);
        }
        // base class
        static int getCameraComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(RenderingComponent,static_cast<CameraComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_FovH(){ return "FovH";}
        static const char* getFieldTypeName_FovH(){ return "float";}
        static void set_FovH(void* instance, void* field_value){ static_cast<CameraComponent*>(instance)->FovH = *static_cast<float*>(field_value);}
        static void* get_FovH(void* instance){ return static_cast<void*>(&(static_cast<CameraComponent*>(instance)->FovH));}
        static bool isArray_FovH(){ return false; }
        static const char* getFieldName_Zoom(){ return "Zoom";}
        static const char* getFieldTypeName_Zoom(){ return "float";}
        static void set_Zoom(void* instance, void* field_value){ static_cast<CameraComponent*>(instance)->Zoom = *static_cast<float*>(field_value);}
        static void* get_Zoom(void* instance){ return static_cast<void*>(&(static_cast<CameraComponent*>(instance)->Zoom));}
        static bool isArray_Zoom(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_CameraComponent(){
        FieldFunctionTuple* field_function_tuple_FovH=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::set_FovH,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::get_FovH,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getFieldName_FovH,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getFieldTypeName_FovH,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::isArray_FovH);
        REGISTER_FIELD_TO_MAP("CameraComponent", field_function_tuple_FovH);
        FieldFunctionTuple* field_function_tuple_Zoom=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::set_Zoom,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::get_Zoom,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getFieldName_Zoom,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getFieldTypeName_Zoom,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::isArray_Zoom);
        REGISTER_FIELD_TO_MAP("CameraComponent", field_function_tuple_Zoom);

        
        
        
        ClassFunctionTuple* class_function_tuple_CameraComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::getCameraComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeCameraComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("CameraComponent", class_function_tuple_CameraComponent);
    }
namespace TypeWrappersRegister{
    void CameraComponent()
    {
        TypeWrapperRegister_CameraComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

