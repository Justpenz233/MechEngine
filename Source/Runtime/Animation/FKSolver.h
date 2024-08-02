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

    template<class T> requires std::is_base_of_v<Joint, T>
    void AddJoint(ObjectPtr<T> InJoint)
    {
    	JointsSet.insert(Cast<Joint>(InJoint));
    }

    void AddJoints(TArray<ObjectPtr<Joint>> InJoint);

	template<class T> requires std::is_base_of_v<Joint, T>
	void AddJoints(TArray<ObjectPtr<T>> InJoint)
	{
		for (auto i : InJoint)
			JointsSet.insert(Cast<Joint>(i));
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