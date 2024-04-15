#pragma once
#include "ContainerTypes.h"

// Ref: https://zhuanlan.zhihu.com/p/663942977
template<typename R, typename... Args>
class VDelegate
{
public:
	virtual R Execute(Args... args) = 0;
	virtual ~VDelegate() {}
};


template<class T, typename R, typename... Args>
class ObjectDelegate : public VDelegate<R, Args...>
{
	using FuncPtr = R(T::*)(Args...);
public:
	ObjectDelegate(T* p, FuncPtr f) : p(p), fp(f) {}

	R Execute(Args... args)
	{
		return (p->*fp)(args...);
	}
private:
	T* p;
	FuncPtr fp;
};

template<typename TLambda, typename R, typename... Args>
class LambdaDelegate : public VDelegate<R, Args...>
{
public:
	LambdaDelegate(TLambda Lambda) : Lambda(Lambda) {}

	R Execute(Args... args)
	{
		return Lambda(args...);
	}
private:
	TLambda Lambda;
};

template<typename R, typename... Args>
class Delegate
{
public:
	Delegate() {}

	template<typename T>
	Delegate(T* P, R(T::* fp)(Args...)) {BindMember(P, fp);}

	template<typename TLambda>
	Delegate(TLambda Lambda) { BindLambda(Lambda); }

	template<typename T>
	void BindMember(T* P, R(T::* fp)(Args...))
	{
        delete del;
		del = new ObjectDelegate<T, R, Args...>(P, fp);
	}
	
    template<typename TLambda>
    void BindLambda(TLambda Lambda)
    {
        delete del;
        del = new LambdaDelegate<TLambda, R, Args...>(Lambda);
    }

	bool IsBound()
	{
		return del != nullptr;
	}

	R Execute(Args... args)
	{
		ASSERTMSG(IsBound(), "Delegate not bound");
		return del->Execute(args...);
	}

	R ExecuteIfBound(Args...args)
	{
		if(IsBound())
			return del->Execute(args...);
	}
private:
	VDelegate<R, Args...>* del = nullptr;
};


template<typename... Args>
class MultiCastDelegate
{
public:
	MultiCastDelegate(){}
	~MultiCastDelegate(){}

	template<class T>
	void AddMember(T* Object, void(T::* FuncPtr)(Args ...))
	{
		Delegate<void, Args...> delegate;
		delegate.BindMember(Object, FuncPtr);
		DelegateList.push_back(delegate);
	}

	template<class T>
	void AddLambda(T Lambda)
	{
		Delegate<void, Args...> delegate;
		delegate.BindLambda(Lambda);
		DelegateList.push_back(delegate);
	}

	template<class T>
	void AddObjectFunction(T* Ptr, void(T::* fp)(Args...))
	{
		Delegate<void, Args...> delegate;
		delegate.BindMember(Ptr, fp);
		DelegateList.push_back(delegate);
	}

	inline void Broadcast(Args... args)
	{
		for(auto delegate : DelegateList)
		{
			delegate.ExecuteIfBound(args...);
		}
	}

protected:
	TArray<Delegate<void, Args...>> DelegateList;
};
#define DECLARE_DELEGATE(DelegateName) typedef Delegate<void> DelegateName;

#define DECLARE_DELEGATE_PARMS(DelegateName, ...) typedef Delegate<void, __VA_ARGS__> DelegateName;

#define DECLARE_MULTICAST_DELEGATE(DelegateName) typedef MultiCastDelegate<> DelegateName;

#define DECLARE_MULTICAST_DELEGATE_PARMS(DelegateName, ...) typedef MultiCastDelegate<__VA_ARGS__> DelegateName;

#define DECLARE_DELEGATE_RET(ReturnType, DelegateName) typedef Delegate<ReturnType> DelegateName;

#define DECLARE_DELEGATE_RET_PARMS(ReturnType, DelegateName, ...) typedef Delegate<ReturnType, __VA_ARGS__> DelegateName;