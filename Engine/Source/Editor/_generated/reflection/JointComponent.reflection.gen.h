#pragma once
#include "Animation/JointComponent.h"

class JointComponent;
class IKJointComponent;
namespace Reflection{
namespace TypeFieldReflectionOparator{
    class TypeJointComponentOperator{
    public:
        static const char* getClassName(){ return "JointComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(JointComponent);
            JointComponent* ret_instance= new JointComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(JointComponent*)instance);
        }
        // base class
        static int getJointComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(StaticMeshComponent,static_cast<JointComponent*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_JointComponent(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_JointComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeJointComponentOperator::getJointComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeJointComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeJointComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("JointComponent", class_function_tuple_JointComponent);
    }namespace TypeFieldReflectionOparator{
    class TypeIKJointComponentOperator{
    public:
        static const char* getClassName(){ return "IKJointComponent";}
        static void* constructorWithJson(const Json& json_context){
            CheckDefaultConstructible(IKJointComponent);
            IKJointComponent* ret_instance= new IKJointComponent();
            Serializer::read(json_context, *ret_instance);
            return ret_instance;
        }
        static Json writeByName(void* instance){
            return Serializer::write(*(IKJointComponent*)instance);
        }
        // base class
        static int getIKJointComponentBaseClassReflectionInstanceList(ReflectionInstance* &out_list, void* instance){
            int count = 1;
            out_list = new ReflectionInstance[count];
            int i = 0;
               out_list[i++] = TypeMetaDef(JointComponent,static_cast<IKJointComponent*>(instance));
            return count;
        }
        // fields
        

        // methods
        
    };
}//namespace TypeFieldReflectionOparator


    void TypeWrapperRegister_IKJointComponent(){
        

        
        
        
        ClassFunctionTuple* class_function_tuple_IKJointComponent=new ClassFunctionTuple(
            &TypeFieldReflectionOparator::TypeIKJointComponentOperator::getIKJointComponentBaseClassReflectionInstanceList,
            &TypeFieldReflectionOparator::TypeIKJointComponentOperator::constructorWithJson,
            &TypeFieldReflectionOparator::TypeIKJointComponentOperator::writeByName);
        REGISTER_BASE_CLASS_TO_MAP("IKJointComponent", class_function_tuple_IKJointComponent);
    }
namespace TypeWrappersRegister{
    void JointComponent()
    {
        TypeWrapperRegister_JointComponent();
    TypeWrapperRegister_IKJointComponent();
    }
}//namespace TypeWrappersRegister
}//namespace Reflection

