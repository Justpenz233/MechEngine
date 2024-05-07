//
// Created by MarvelLi on 2024/1/30.
//

#include "GeometryProcess.h"
#include "Mesh/StaticMesh.h"
#include "igl/swept_volume.h"
#include "Math/FTransform.h"
#include "igl/vertex_components.h"

#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/border.h>

namespace MechEngine::Algorithm::GeometryProcess
{
	bool FillSmallestHole(Eigen::MatrixX3d& verM, Eigen::MatrixX3i& triM)
	{
		return true;
	}

	bool FillAllHoles(Eigen::MatrixX3d& verM, Eigen::MatrixX3i& triM)
	{
		typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
		typedef Kernel::Point_3										Point_3;
		typedef std::array<int, 3>									Facet;
		typedef CGAL::Surface_mesh<Point_3>							Mesh_;
		typedef boost::graph_traits<Mesh_>::vertex_descriptor		vertex_descriptor;
		typedef boost::graph_traits<Mesh_>::halfedge_descriptor		halfedge_descriptor;
		typedef boost::graph_traits<Mesh_>::face_descriptor			face_descriptor;
		namespace PMP = CGAL::Polygon_mesh_processing;

		std::vector<Point_3> vertices(verM.rows());
		for (int i = 0; i < verM.rows(); i++)
		{
			vertices[i] = { verM(i, 0), verM(i, 1), verM(i, 2) };
		}
		std::vector<Facet> facets(triM.rows());
		for (int i = 0; i < triM.rows(); i++)
		{
			facets[i] = { triM(i, 0), triM(i, 1), triM(i, 2) };
		}
		CGAL::Surface_mesh<Point_3> mesh;
		CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(vertices, facets, mesh);

		std::vector<halfedge_descriptor> border_cycles;
		// collect one halfedge per boundary cycle
		PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));
		bool Success = false;
		for (halfedge_descriptor h : border_cycles)
		{
			std::vector<face_descriptor>   patch_facets;
			std::vector<vertex_descriptor> patch_vertices;
			PMP::triangulate_and_refine_hole(mesh, h,
			CGAL::parameters::face_output_iterator(std::back_inserter(patch_facets))
			.vertex_output_iterator(std::back_inserter(patch_vertices)));
			Success = true;
		}

		verM.resize(mesh.num_vertices(), 3);
		triM.resize(mesh.num_faces(), 3);
		int i = 0;
		for (const Point_3& p : mesh.points())
		{
			for (int j = 0; j < 3; j++)
				verM(i, j) = p[j];
			i++;
		}
		i = 0;

		BOOST_FOREACH (face_descriptor f, faces(mesh))
		{
			int										 j = 0;
			CGAL::Vertex_around_face_iterator<Mesh_> vbegin, vend;
			std::vector<int>						 fc;
			for (boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(f), mesh);
				 vbegin != vend;
				 ++vbegin)
			{
				j++;
				fc.emplace_back(*vbegin);
			}
			for (int k = 0; k < 3; k++)
				triM(i, k) = fc[k];
			i++;
		}
		return Success;
	}

	void SmoothMesh(Eigen::MatrixX3d& verM, Eigen::MatrixX3i& triM, int Iteration, bool UseUniformLaplacian)
	{

	}


	ObjectPtr<StaticMesh> SweptVolume(const ObjectPtr<StaticMesh>& Profile, const TArray<FTransform>& Path, int Steps)
	{
		MatrixXd RV; MatrixXi RF;
		if(Steps == -1)
			Steps = Path.size();
		igl::swept_volume(Profile->verM, Profile->triM, [&Path](const double t)->Eigen::Affine3d
		{
			int index = std::floor(t * (Path.size() - 1.));
			double Alph = t * (Path.size() - 1.) - index;
			int NextIndex = Math::Min(index + 1, Path.size() - 1);
			return Eigen::Affine3d(FTransform::Lerp(Path[index], Path[NextIndex], Alph));
		}, Steps, 100, 0, RV, RF);
		return NewObject<StaticMesh>(::std::move(RV), ::std::move(RF));
	}


	TArray<ObjectPtr<StaticMesh>> DivideMeshIntoComponents(const ObjectPtr<StaticMesh>& Mesh)
	{
		Eigen::ArrayXi VertexComponent;
		igl::vertex_components(Mesh->triM, VertexComponent);
		int ComponentCount = VertexComponent.maxCoeff() + 1;
		if (ComponentCount == 1)
			return { NewObject<StaticMesh>(*Mesh) };
		TMap<int, int>	  VertexMap; // Map from old vertex index to new vertex index
		TArray<TArray<FVector>>  verM(ComponentCount);
		TArray<TArray<Vector3i>> Indicies(ComponentCount);
		for (int i = 0; i < Mesh->verM.rows(); ++i)
		{
			int ComponentIndex = VertexComponent(i);
			VertexMap[i] = verM[ComponentIndex].size();
			verM[ComponentIndex].emplace_back(Mesh->verM.row(i));
		}
		for (int i = 0; i < Mesh->triM.rows(); ++i)
		{
			Vector3i Tri = Mesh->triM.row(i);
			int		 ComponentIndex = VertexComponent(Tri(0));
			for (int j = 0; j < 3; ++j)
				Tri(j) = VertexMap[Tri(j)];
			Indicies[ComponentIndex].emplace_back(Tri);
		}
		TArray<ObjectPtr<StaticMesh>> Result;
		for (int i = 0; i < ComponentCount; ++i)
		{
			Result.push_back(NewObject<StaticMesh>(verM[i], Indicies[i]));
		}
		return std::move(Result);
	}

	int MeshComponentsCount(const ObjectPtr<StaticMesh>& Mesh)
	{
		Eigen::ArrayXi VertexComponent;
		igl::vertex_components(Mesh->triM, VertexComponent);
		return VertexComponent.maxCoeff() + 1;
	}

	} // namespace MechEngine::Algorithm::GeometryProcess