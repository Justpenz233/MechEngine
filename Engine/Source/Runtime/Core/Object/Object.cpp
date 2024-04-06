//
// Created by MarvelLi on 2023/8/23.
//

#include "Object.h"
#include <set>
#include "Log/Log.h"

Object::Object(const std::string& InObjectName): ObjectName(InObjectName) { }

Object::Object(std::string&& InObjectName) : ObjectName(std::move(InObjectName)) { }

// TODO: consider thread safe
void Object::RegisterObject()
{
    // if(ObjectName.empty())
        // ObjectName = AssignObjectName(this);
    // GObjectMap[ObjectName] = this;
}

Object::~Object()
{
    // GObjectMap.erase(ObjectName);
}

Reflection::ReflectionInstance Object::GetMetaInfo(const Class& ClassName, void* Instance)
{
    return {Reflection::TypeMeta::newMetaFromName(ClassName), Instance};
}

std::vector<Object::Class> Object::GetBaseClassDirect()
{
    auto meta = GetMetaInfo();
    Reflection::ReflectionInstance* list;
    int Count = meta.m_meta.getBaseClassReflectionInstanceList(list, this);
    std::vector<Object::Class>  ret;
    for(int i = 0;i < Count;i ++) {
        ret.emplace_back(list[i].m_meta.getTypeName());
    }
    return ret;
}

std::vector<Object::Class> Object::GetBaseClassRecursive()
{
    std::function<std::set<std::string>(Class CurrentClass)> RecursiveBase;
    RecursiveBase = [this, &RecursiveBase] (const Object::Class& CurrentClass)
    {
        std::set<std::string> ret;
        auto meta = Object::GetMetaInfo(CurrentClass, this);
        Reflection::ReflectionInstance* list;
        int Count = meta.m_meta.getBaseClassReflectionInstanceList(list, this);
        for(int i = 0;i < Count;i ++) {
            ret.insert(list[i].m_meta.getTypeName());
            auto temp = RecursiveBase(list[i].m_meta.getTypeName());
            ret.insert(temp.begin(), temp.end());
        }
        return ret;
    };
    auto Ret = RecursiveBase(ClassName());
    return {Ret.begin(), Ret.end()};

}

std::vector<Reflection::FieldAccessor> Object::GetAllPropertyAceessors(EAccessorFlag Flag)
{
    std::function<void(Class CurrentClass)> RecursiveBase;
    std::vector<Reflection::FieldAccessor> ret;

    // Get this propery
    auto ThisMeta = Object::GetMetaInfo(ClassName(), this);
    auto ThisProperty = ThisMeta.m_meta.getFieldsList();
    ret.insert(ret.end(), ThisProperty.begin(), ThisProperty.end());

    if (Flag != ExculdeParent) {
        for(auto ParaentClass : GetBaseClassRecursive()) {
            auto Meta = Object::GetMetaInfo(ParaentClass, this);
            auto Property = Meta.m_meta.getFieldsList();
            ret.insert(ret.end(), Property.begin(), Property.end());
        }
    }
    return ret;
}

std::vector<Reflection::MethodAccessor> Object::GetAllMethodAceessors(EAccessorFlag Flag)
{
    std::function<void(Class CurrentClass)> RecursiveBase;
    std::vector<Reflection::MethodAccessor> ret;

    // Get this propery
    auto ThisMeta = Object::GetMetaInfo(ClassName(), this);
    auto ThisMethod = ThisMeta.m_meta.getMethodsList();
    ret.insert(ret.end(), ThisMethod.begin(), ThisMethod.end());

    if (Flag != ExculdeParent) {
        for(auto ParaentClass : GetBaseClassRecursive()) {
            auto Meta = Object::GetMetaInfo(ParaentClass, this);
            auto Property = Meta.m_meta.getMethodsList();
            ret.insert(ret.end(), Property.begin(), Property.end());
        }
    }
    return ret;
}

std::string Object::AssignObjectName(Object* InObject)
{
	//TODO
    std::string AssignName = InObject->ClassName();
    return AssignName;
}