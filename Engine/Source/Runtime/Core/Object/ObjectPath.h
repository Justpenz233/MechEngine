//
// Created by MarvelLi on 2024/2/26.
//

#pragma once
#include "Core/CoreMinimal.h"

// This is basic class for Object path
// Each object needed to be serialized should be contained in a package and have a object name.
// The path is formatted as "PackagePath/ObjectName.ObjectClassName"
class ObjectPath
{
public:
	ObjectPath() = default;

private:
	String PackagePath;
	String ObjectName;
	String ObjectClassName;
	WeakObjectPtr<Object> Ptr;
};
