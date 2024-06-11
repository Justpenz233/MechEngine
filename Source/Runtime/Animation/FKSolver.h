#pragma once
#include "Core/CoreMinimal.h"
#include "Animation/Joints.h"
class ENGINE_API FKSolver : public Object
{
protected:
    std::set<ObjectPtr<Joint>> JointsSet;
    TArray<ObjectPtr<Joint>> Joints;
	TArray<ObjectPtr<Joint>> Roots;
    void AddJoints(){}
public:
    FKSolver();
	FKSolver(TArray<ObjectPtr<Joint>>&& InJoints);
    ~FKSolver();

    virtual void SortJoints();

    template<class T>
    void AddJoint(ObjectPtr<T> InJoint)
    {
        if(InJoint->template IsSubclassOf<Joint>())
            JointsSet.insert(Cast<Joint>(InJoint));
    }

    void AddJoints(TArray<ObjectPtr<Joint>> InJoint);

	template<class T>
	void AddJoints(TArray<ObjectPtr<T>> InJoint)
	{
		for (auto i : InJoint)
			if(auto JointA = Cast<Joint>(i))
				JointsSet.insert(JointA);
	}

    template<class T, typename... Args>
    void AddJoints(T InJoint, Args... args)
    {
        if(auto JointA = Cast<Joint>(InJoint))
            JointsSet.insert(JointA);
        else
            LOG_WARNING("Non-Joint object trying add into solver");
        AddJoints(args...);
    }

	virtual void Init();
	virtual double Solve();

    // Interface to drive a root joint with delta time
    virtual void Drive(ObjectPtr<Joint> Root, double DeltaTime)
    {
        static_assert("Not Implemented");
    }

    TArray<ObjectPtr<Joint>> FindRoots();
};