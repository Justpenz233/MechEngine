//
// Created by MarvelLi on 2023/8/23.
//

#pragma once

#include "PointerTypes.h"
#include "Delegate.h"
#include "ObjectInitiliazer.h"
#include "Reflection/reflection/reflection.h"

DECLARE_MULTICAST_DELEGATE_PARMS(FOnPostEdit, Reflection::FieldAccessor&)
DECLARE_MULTICAST_DELEGATE_PARMS(FOnPreEdit, Reflection::FieldAccessor&)

enum EAccessorFlag
{
	// Include all propertys recursively from parent class
	Default,

	// Exclude all property from parent class
	ExculdeParent
};

/**
 *  Object is a base class for all objects in the engine, especially for game threads.
 *  Object is capable of serialization, reflection, and garbage collection. All the class acts in world should be inherited from Object.
 */
MCLASS(Object)
class Object : public std::enable_shared_from_this<Object>
{
	REFLECTION_BODY_OBJECT(Object)
public:
    Object() = default;
    virtual ~Object();

	// Set object name
    FORCEINLINE void SetName(const String& InName)
    {
    	ObjectName = InName;
    }

	// Get object name
	FORCEINLINE String GetName() const
	{
    	return ObjectName;
	}

	// Get shared pointer of this object
    FORCEINLINE ObjectPtr<Object> GetThis()
    {
	    return shared_from_this();
    }
    
    template<class T>
    FORCEINLINE ObjectPtr<T> GetThis();

//    TODO: serilization
    virtual bool Load() { return true; }
    virtual bool Save() { return true; }

	// Called before the Filed is edited
	FORCEINLINE virtual void PreEdit(Reflection::FieldAccessor& Field)
    {
    	PreEditDelegate.Broadcast(Field);
    }

	FORCEINLINE FOnPreEdit& GetPreEditDelegate()
	{
		return PreEditDelegate;
	}

	// Called after the Filed is edited
	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field)
    {
	    PostEditDelegate.Broadcast(Field);
    }

	FORCEINLINE FOnPostEdit& GetPostEditDelegate()
	{
		return PostEditDelegate;
	}

	/// \brief Is this object is a instance of class T
	/// \tparam T Class type to test
	/// \return true if this object is a instance of T
	template <class T>
	FORCEINLINE bool IsA();

	/// \brief Is this object is a subclass of class T. include Is T
	/// \tparam T Class type to test
	/// \return true if this object is a subclass of T
	template <class T>
	FORCEINLINE bool IsSubclassOf();

	// Reflection
	typedef String Class;

	// Get immediate parent class
	std::vector<Class> GetBaseClassDirect();

	// Get all parent classes recursively
	std::vector<Class> GetBaseClassRecursive();

	// Get all MPROPERTY from this class (and parent class)
	std::vector<Reflection::FieldAccessor> GetAllPropertyAceessors(EAccessorFlag Flag = EAccessorFlag::Default);

	// Get all MFUNCTION from this class (and parent class)
	std::vector<Reflection::MethodAccessor> GetAllMethodAceessors(EAccessorFlag Flag = EAccessorFlag::Default);

	template<typename RetValue = void, typename... Args>
	auto InvokeFunction(const String& MethodName, Args&&... Parameter) -> std::conditional_t<std::is_void<RetValue>::value, void, RetValue>
	{
		auto meta = GetMetaInfo();
		auto Method = meta.m_meta.getMethodByName(MethodName);

		if constexpr (std::is_void<RetValue>::value) {
			Method.Invoke(this, std::forward<Args>(Parameter)...);
		} else {
			return std::any_cast<RetValue>(Method.Invoke(this, std::forward<Args>(Parameter)...));
		}
	}

	FORCEINLINE Reflection::ReflectionInstance GetMetaInfo()
	{
		return GetMetaInfo(ClassName(), this);
	}

	static Reflection::ReflectionInstance GetMetaInfo(const Class& ClassName, void* Instance);

protected:
	MPROPERTY()
	String ObjectName;

	// Object this object is contained in
	Object* Outer = nullptr;

	FOnPostEdit PostEditDelegate;
	FOnPreEdit PreEditDelegate;
	void RegisterObject();

	static String AssignObjectName(Object*);

	friend class ObjectFactory;

	Object(const String& InObjectName);
	Object(String&& InObjectName);
};

template <class T>
concept IsObject = std::is_base_of_v<Object, T>;

template <class T, class... Args>
concept ObjectConstructBy = IsObject<T> && std::constructible_from<T, Args...>;

template <class T>
ObjectPtr<T> Object::GetThis()
{
	return Cast<T>(shared_from_this());
}
template <class T>
bool Object::IsA()
{
	return (typeid(*this) == typeid(T));
}

template <class T>
bool Object::IsSubclassOf()
{
	return (Cast<T>(shared_from_this()) != nullptr);
}

template <class T, typename... Args>
ObjectPtr<T> NewObject(Args &&...args)
{
	static_assert(IsObject<T>, " must be a subclass of Object");
	static_assert(!std::is_abstract_v<T>, "T is a abstract class, please check the base class of T");
	static_assert(std::constructible_from<T, Args...>, "T must be constructible by Args, please check the constructor of T");
    return std::make_shared<T>(std::forward<Args>(args)...);
}