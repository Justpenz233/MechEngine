#pragma once

#include "Core/CoreMinimal.h"
#include "Mesh/StaticMesh.h"

enum BooleanType
{
    UNION, INTERSECTION, A_NOT_B
};

class ENGINE_API MeshBoolean {

public:

    static ObjectPtr<StaticMesh> MeshConnect(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B );

    static ObjectPtr<StaticMesh> MeshMinus(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B);

    static ObjectPtr<StaticMesh> MeshUnion(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B);

	static ObjectPtr<StaticMesh> MeshIntersect(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B);
	
    static ObjectPtr<StaticMesh> Boolean(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B, BooleanType type);
};