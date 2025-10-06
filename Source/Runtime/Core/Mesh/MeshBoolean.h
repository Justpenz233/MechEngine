#pragma once

#include "Core/CoreMinimal.h"
#include "Mesh/StaticMesh.h"

enum BooleanType
{
    UNION, INTERSECTION, A_NOT_B
};

namespace MeshBoolean {

    ENGINE_API ObjectPtr<StaticMesh> MeshConnect(const ObjectPtr<StaticMesh>& A, const ObjectPtr<StaticMesh>& B );

    ENGINE_API ObjectPtr<StaticMesh> MeshMinus(const ObjectPtr<StaticMesh>& A, const ObjectPtr<StaticMesh>& B);

    ENGINE_API ObjectPtr<StaticMesh> MeshUnion(const ObjectPtr<StaticMesh>& A, const ObjectPtr<StaticMesh>& B);

	ENGINE_API ObjectPtr<StaticMesh> MeshIntersect(const ObjectPtr<StaticMesh>& A, const ObjectPtr<StaticMesh>& B);
	
    ENGINE_API ObjectPtr<StaticMesh> Boolean(const ObjectPtr<StaticMesh>& A, const ObjectPtr<StaticMesh>& B, BooleanType type);
};