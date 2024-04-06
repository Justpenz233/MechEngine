//
// Created by MarvelLi on 2024/3/28.
//


#pragma once
#include <luisa/core/basic_traits.h>

namespace MechEngine::Rendering
{

	struct instanceData
	{
		// Vertex buffer id in bindless array
		uint VertexID = -1;

		// Triangle id in bindless array
		uint TriangleID = -1;
	};

}

LUISA_STRUCT(MechEngine::Rendering::instanceData, VertexID, TriangleID) {};