#include "MeshBoolean.h"
#include "StaticMesh.h"
#include "Object/Object.h"

#include "igl/MeshBooleanType.h"
#include "igl/copyleft/cgal/mesh_boolean.h"

#if defined(_MSC_VER)
	#pragma warning(disable : 4267)
#endif

///==============================================================================//
///                              Boolean
///==============================================================================//

ObjectPtr<StaticMesh> MeshBoolean::MeshMinus(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B)
{
    return Boolean(A, B, BooleanType::A_NOT_B);
}

ObjectPtr<StaticMesh> MeshBoolean::MeshUnion(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B)
{
	return Boolean(A, B, BooleanType::UNION);
}

ObjectPtr<StaticMesh> MeshBoolean::MeshIntersect(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B)
{
	return Boolean(A, B, BooleanType::INTERSECTION);
}

ObjectPtr<StaticMesh> MeshBoolean::MeshConnect(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B)
{
    MatrixXd VC;
    MatrixXi FC, fB;
    MatrixXd NC, CC;

    long numVertA, numVertB, numFaceA, numFaceB, numNormA, numNormB, numColA, numColB;
    numVertA = A->verM.rows();
    if(numVertA == 0) return B;
    numVertB = B->verM.rows();
    if(numVertB == 0) return A;
    numFaceA = A->triM.rows();
    numFaceB = B->triM.rows();
    numNormA = A->norM.rows();
    numNormB = B->norM.rows();
    numColA  = A->colM.rows();
    numColB  = B->colM.rows();

    VC.resize(numVertA + numVertB, 3);
    FC.resize(numFaceA + numFaceB, 3);
    NC.resize(numNormA + numNormB, 3);
    CC.resize(numColA + numColB, 3);


    VC.block(0, 0, numVertA, 3) = A->verM;
    FC.block(0, 0, numFaceA, 3) = A->triM;
    CC.block(0, 0, numColA, 3)  = A->colM;
    NC.block(0, 0, numNormA, 3) = A->norM;

    VC.block(numVertA, 0, numVertB, 3) = B->verM;
    CC.block(numColA, 0, numColB, 3)   = B->colM;
    NC.block(numNormA, 0, numNormB, 3) = B->norM;

    fB.resize(numFaceB, 3);
    fB.setOnes();
    fB = fB * numVertA;
    FC.block(numFaceA, 0, numFaceB, 3) = B->triM + fB;

    return NewObject<StaticMesh>(std::move(VC), std::move(FC));
}
// std::ObjectPtr<StaticMesh> BooleanMcut(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B, BooleanType type)
// {
// 	std::string boolOpStr;
//
// 	if (type == BooleanType::UNION)
// 	{
// 		boolOpStr = "UNION";
// 	}
// 	else if (type == BooleanType::INTERSECTION)
// 	{
// 		boolOpStr = "INTERSECTION";
// 	}
// 	else if (type == BooleanType::A_NOT_B)
// 	{
// 		boolOpStr = "A_NOT_B";
// 	}
// 	else if (type == BooleanType::B_NOT_A)
// 	{
// 		boolOpStr = "B_NOT_A";
// 	}
// 	else
// 	{
// 		fprintf(stderr, "invalid boolOp argument value\n");
// 		return {};
// 	}
//
// 	McContext context = MC_NULL_HANDLE;
// 	McSize	  numBytes = 0;
//
// 	McResult err = mcCreateContext(&context, false);
// 	mcDebugMessageControl(context, McDebugSource::MC_DEBUG_SOURCE_ALL,
// 		McDebugType::MC_DEBUG_TYPE_ALL,
// 		McDebugSeverity::MC_DEBUG_SEVERITY_ALL,
// 		false); // <---- here is what you want}
// 	assert(err == MC_NO_ERROR);
//
// 	std::map<std::string, McFlags> booleanOps = {
// 		{ "A_NOT_B", MC_DISPATCH_FILTER_FRAGMENT_SEALING_INSIDE | MC_DISPATCH_FILTER_FRAGMENT_LOCATION_ABOVE },
// 		{ "B_NOT_A", MC_DISPATCH_FILTER_FRAGMENT_SEALING_OUTSIDE | MC_DISPATCH_FILTER_FRAGMENT_LOCATION_BELOW },
// 		{ "UNION", MC_DISPATCH_FILTER_FRAGMENT_SEALING_OUTSIDE | MC_DISPATCH_FILTER_FRAGMENT_LOCATION_ABOVE },
// 		{ "INTERSECTION", MC_DISPATCH_FILTER_FRAGMENT_SEALING_INSIDE | MC_DISPATCH_FILTER_FRAGMENT_LOCATION_BELOW }
// 	};
//
// 	const McFlags boolOpFlags = booleanOps[boolOpStr];
//
// 	// Config Commands  TODO: change to enum
// 	// -----------------------------------------
// 	MatrixXd AVert = A->verM.transpose(), BVert = B->verM.transpose();
// 	MatrixXi AFace = A->triM.transpose(), BFace = B->triM.transpose();
//
// 	err = mcDispatch(
// 		context,
// 		MC_DISPATCH_VERTEX_ARRAY_DOUBLE |		   // vertices are in array of doubles
// 			MC_DISPATCH_ENFORCE_GENERAL_POSITION | // perturb if necessary
// 			boolOpFlags,						   // filter flags which specify the type of output we want
// 		// A mesh
// 		reinterpret_cast<const void*>(AVert.data()),
// 		reinterpret_cast<const uint32_t*>(AFace.data()),
// 		nullptr,
// 		static_cast<uint32_t>(A->GetVertexNum()),
// 		static_cast<uint32_t>(A->GetFaceNum()),
// 		// B mesh
// 		reinterpret_cast<const void*>(BVert.data()),
// 		reinterpret_cast<const uint32_t*>(BFace.data()),
// 		nullptr,
// 		static_cast<uint32_t>(B->GetVertexNum()),
// 		static_cast<uint32_t>(B->GetFaceNum()));
//
// 	assert(err == MC_NO_ERROR);
//
// 	uint32_t numConnComps;
// 	err = mcGetConnectedComponents(context, MC_CONNECTED_COMPONENT_TYPE_FRAGMENT, 0, NULL, &numConnComps);
// 	assert(err == MC_NO_ERROR);
// 	std::vector<McConnectedComponent> connectedComponents(numConnComps, MC_NULL_HANDLE);
// 	connectedComponents.resize(numConnComps);
// 	err = mcGetConnectedComponents(context, MC_CONNECTED_COMPONENT_TYPE_FRAGMENT, (uint32_t)connectedComponents.size(), connectedComponents.data(), NULL);
// 	assert(err == MC_NO_ERROR);
//
// 	auto Result = NewObject<StaticMesh>();
// 	for (int i = 0; i < connectedComponents.size(); i++)
// 	{
// 		McConnectedComponent connComp = connectedComponents[i];
// 		McSize				 numBytes = 0; // Size of vertex position list
// 		err = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_VERTEX_DOUBLE, 0, NULL, &numBytes);
// 		assert(err == MC_NO_ERROR);
// 		uint32_t			VertexCount = (uint32_t)(numBytes / (sizeof(double) * 3));
// 		std::vector<double> Vertices((McSize)VertexCount * 3u, 0);
// 		err = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_VERTEX_DOUBLE, numBytes, (void*)Vertices.data(), NULL);
// 		assert(err == MC_NO_ERROR);
//
// 		err = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_FACE_TRIANGULATION, 0, NULL, &numBytes);
// 		assert(err == MC_NO_ERROR);
// 		std::vector<uint32_t> FaceIndices(numBytes / sizeof(uint32_t), 0);
// 		err = mcGetConnectedComponentData(context, connComp, MC_CONNECTED_COMPONENT_DATA_FACE_TRIANGULATION, numBytes, FaceIndices.data(), NULL);
// 		assert(err == MC_NO_ERROR);
//
// 		const uint32_t FaceCount = FaceIndices.size() / 3;
//
// 		MatrixXd Vertex;
// 		Vertex.resize(3, VertexCount);
// 		memcpy(Vertex.data(), Vertices.data(), Vertices.size() * sizeof(double));
// 		Vertex.transposeInPlace();
//
// 		MatrixXi Face;
// 		Face.resize(3, FaceCount);
// 		memcpy(Face.data(), FaceIndices.data(), FaceIndices.size() * sizeof(int));
// 		Face.transposeInPlace();
// 		auto temp = NewObject<StaticMesh>(Vertex, Face);
// 		Result = MeshBoolean::MeshConnect(Result, temp);
// 	}
//
// 	mcReleaseConnectedComponents(context, 0, NULL);
// 	mcReleaseContext(context);
//
// 	return Result;
// }

ObjectPtr<StaticMesh> BooleanCGAL(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B, BooleanType type)
{
	MatrixXd V;
	MatrixXi F;
	if (type == BooleanType::UNION)
	{
		igl::copyleft::cgal::mesh_boolean(A->verM, A->triM, B->verM, B->triM, igl::MESH_BOOLEAN_TYPE_UNION, V, F);
	}
	else if (type == BooleanType::INTERSECTION)
	{
		igl::copyleft::cgal::mesh_boolean(A->verM, A->triM, B->verM, B->triM, igl::MESH_BOOLEAN_TYPE_INTERSECT, V, F);
	}
	else if (type == BooleanType::A_NOT_B)
	{
		igl::copyleft::cgal::mesh_boolean(A->verM, A->triM, B->verM, B->triM, igl::MESH_BOOLEAN_TYPE_MINUS, V, F);
	}
	return NewObject<StaticMesh>(std::move(V), std::move(F));
}

ObjectPtr<StaticMesh> MeshBoolean::Boolean(ObjectPtr<StaticMesh> A, ObjectPtr<StaticMesh> B, BooleanType type)
{
    return BooleanCGAL(A, B, type);
}