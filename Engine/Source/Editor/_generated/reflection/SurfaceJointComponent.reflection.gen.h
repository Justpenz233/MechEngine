#pragma once
#include "../../../DevelopableMechanism/Source/SpacialMechanism/SurfaceJointComponent.h"

class SurfaceJointComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeSurfaceJointComponentOperator{
    public:
        static const char* getClassName(){ return "SurfaceJointComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(SurfaceJointComponent);
            SurfaceJointComponent* ret_instance= new SurfaceJointComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(SurfaceJointComponent*)instance);
        }
        // base class
        static int getSurfaceJointComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(IKJointComponent,static_cast<SurfaceJointComponent*>(instance));
            return count;
        }
        // fields
        static const char* getFieldName_TransitLinkageLength(){ return "TransitLinkageLength";}
        static const char* getFieldTypeName_TransitLinkageLength(){ return "double";}
        static void set_TransitLinkageLength(void* instance, void* field_value){ static_cast<SurfaceJointComponent*>(instance)->TransitLinkageLength = *static_cast<double*>(field_value);}
        static void* get_TransitLinkageLength(void* instance){ return static_cast<void*>(&(static_cast<SurfaceJointComponent*>(instance)->TransitLinkageLength));}
        static bool isArray_TransitLinkageLength(){ return false; }
        static const char* getFieldName_SocketPortIndex(){ return "SocketPortIndex";}
        static const char* getFieldTypeName_SocketPortIndex(){ return "int";}
        static void set_SocketPortIndex(void* instance, void* field_value){ static_cast<SurfaceJointComponent*>(instance)->SocketPortIndex = *static_cast<int*>(field_value);}
        static void* get_SocketPortIndex(void* instance){ return static_cast<void*>(&(static_cast<SurfaceJointComponent*>(instance)->SocketPortIndex));}
        static bool isArray_SocketPortIndex(){ return false; }
        static const char* getFieldName_JointPortIndex(){ return "JointPortIndex";}
        static const char* getFieldTypeName_JointPortIndex(){ return "int";}
        static void set_JointPortIndex(void* instance, void* field_value){ static_cast<SurfaceJointComponent*>(instance)->JointPortIndex = *static_cast<int*>(field_value);}
        static void* get_JointPortIndex(void* instance){ return static_cast<void*>(&(static_cast<SurfaceJointComponent*>(instance)->JointPortIndex));}
        static bool isArray_JointPortIndex(){ return false; }
        static const char* getFieldName_JointPortLength(){ return "JointPortLength";}
        static const char* getFieldTypeName_JointPortLength(){ return "double";}
        static void set_JointPortLength(void* instance, void* field_value){ static_cast<SurfaceJointComponent*>(instance)->JointPortLength = *static_cast<double*>(field_value);}
        static void* get_JointPortLength(void* instance){ return static_cast<void*>(&(static_cast<SurfaceJointComponent*>(instance)->JointPortLength));}
        static bool isArray_JointPortLength(){ return false; }
        static const char* getFieldName_SocketPortLength(){ return "SocketPortLength";}
        static const char* getFieldTypeName_SocketPortLength(){ return "double";}
        static void set_SocketPortLength(void* instance, void* field_value){ static_cast<SurfaceJointComponent*>(instance)->SocketPortLength = *static_cast<double*>(field_value);}
        static void* get_SocketPortLength(void* instance){ return static_cast<void*>(&(static_cast<SurfaceJointComponent*>(instance)->SocketPortLength));}
        static bool isArray_SocketPortLength(){ return false; }

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_SurfaceJointComponent(){
        FieldFunctionTuple* field_function_tuple_TransitLinkageLength=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::set_TransitLinkageLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::get_TransitLinkageLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldName_TransitLinkageLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldTypeName_TransitLinkageLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::isArray_TransitLinkageLength);
        REGISTER_FIELD_TO_MAP("SurfaceJointComponent", field_function_tuple_TransitLinkageLength);
        FieldFunctionTuple* field_function_tuple_SocketPortIndex=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::set_SocketPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::get_SocketPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldName_SocketPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldTypeName_SocketPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::isArray_SocketPortIndex);
        REGISTER_FIELD_TO_MAP("SurfaceJointComponent", field_function_tuple_SocketPortIndex);
        FieldFunctionTuple* field_function_tuple_JointPortIndex=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::set_JointPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::get_JointPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldName_JointPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldTypeName_JointPortIndex,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::isArray_JointPortIndex);
        REGISTER_FIELD_TO_MAP("SurfaceJointComponent", field_function_tuple_JointPortIndex);
        FieldFunctionTuple* field_function_tuple_JointPortLength=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::set_JointPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::get_JointPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldName_JointPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldTypeName_JointPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::isArray_JointPortLength);
        REGISTER_FIELD_TO_MAP("SurfaceJointComponent", field_function_tuple_JointPortLength);
        FieldFunctionTuple* field_function_tuple_SocketPortLength=new FieldFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::set_SocketPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::get_SocketPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getClassName,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldName_SocketPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getFieldTypeName_SocketPortLength,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::isArray_SocketPortLength);
        REGISTER_FIELD_TO_MAP("SurfaceJointComponent", field_function_tuple_SocketPortLength);

        
        
        
        ClassFunctionTuple* class_function_tuple_SurfaceJointComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::getSurfaceJointComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeSurfaceJointComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("SurfaceJointComponent", class_function_tuple_SurfaceJointComponent);
    }
namespace TypeWrappersRegister{
    void SurfaceJointComponent()
    {
        TypeWrapperRegister_SurfaceJointComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

