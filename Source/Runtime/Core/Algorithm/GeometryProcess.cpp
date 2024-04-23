//
// Created by MarvelLi on 2024/1/30.
//

#include "GeometryProcess.h"
#include "../Mesh/StaticMesh.h"
#include <pmp/algorithms/hole_filling.h>
#include "pmp/surface_mesh.h"
#include "pmp/algorithms/numerics.h"
#include "pmp/algorithms/smoothing.h"
#include "igl/swept_volume.h"
#include "Math/FTransform.h"
#include "igl/vertex_components.h"

namespace MechEngine::Algorithm::GeometryProcess
{
	bool FillSmallestHole(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles)
	{
		pmp::SurfaceMesh mesh_;
		pmp::matrices_to_mesh(Vertices, Triangles, mesh_);
		pmp::Halfedge hmin;
		unsigned int  lmin(mesh_.n_halfedges());
		for (auto h : mesh_.halfedges())
		{
			if (mesh_.is_boundary(h))
			{
				pmp::Scalar	  l(0);
				pmp::Halfedge hh = h;
				do
				{
					++l;
					if (!mesh_.is_manifold(mesh_.to_vertex(hh)))
					{
						l += lmin + 42; // make sure this hole is not chosen
						break;
					}
					hh = mesh_.next_halfedge(hh);
				}
				while (hh != h);

				if (l < lmin)
				{
					lmin = l;
					hmin = h;
				}
			}
		}

		// close smallest hole
		if (hmin.is_valid())
		{
			try
			{
				fill_hole(mesh_, hmin);
			}
			catch (const pmp::InvalidInputException& e)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
		MatrixXd V = Vertices;
		MatrixXi F = Triangles;
		pmp::mesh_to_matrices(mesh_, V, F);
		Vertices = V;
		Triangles = F;
		return true;
	}

	bool FillAllHoles(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles)
	{
		bool			 Flag = false;
		pmp::SurfaceMesh mesh_;
		pmp::matrices_to_mesh(Vertices, Triangles, mesh_);
		while (true)
		{
			pmp::Halfedge hmin;
			unsigned int  lmin(mesh_.n_halfedges());
			for (auto h : mesh_.halfedges())
			{
				if (mesh_.is_boundary(h))
				{
					pmp::Scalar	  l(0);
					pmp::Halfedge hh = h;
					do
					{
						++l;
						if (!mesh_.is_manifold(mesh_.to_vertex(hh)))
						{
							l += lmin + 42; // make sure this hole is not chosen
							break;
						}
						hh = mesh_.next_halfedge(hh);
					}
					while (hh != h);

					if (l < lmin)
					{
						lmin = l;
						hmin = h;
					}
				}
			}
			// close smallest hole
			if (hmin.is_valid())
			{
				try
				{
					fill_hole(mesh_, hmin);
				}
				catch (const pmp::InvalidInputException& e)
				{
					break;
				}
				Flag = true;
			}
			else
				break;
		}
		if (Flag)
		{
			MatrixXd V = Vertices;
			MatrixXi F = Triangles;
			pmp::mesh_to_matrices(mesh_, V, F);
			Vertices = V;
			Triangles = F;
		}
		return Flag;
	}

	void SmoothMesh(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles, int Iteration, bool UseUniformLaplacian)
	{
		pmp::SurfaceMesh mesh_;
		pmp::matrices_to_mesh(Vertices, Triangles, mesh_);
		pmp::explicit_smoothing(mesh_, Iteration, UseUniformLaplacian);
		MatrixXd V = Vertices;
		MatrixXi F = Triangles;
		pmp::mesh_to_matrices(mesh_, V, F);
		Vertices = V;
		Triangles = F;
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
		TArray<TArray<FVector>>  Vertices(ComponentCount);
		TArray<TArray<Vector3i>> Indicies(ComponentCount);
		for (int i = 0; i < Mesh->verM.rows(); ++i)
		{
			int ComponentIndex = VertexComponent(i);
			VertexMap[i] = Vertices[ComponentIndex].size();
			Vertices[ComponentIndex].emplace_back(Mesh->verM.row(i));
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
			Result.push_back(NewObject<StaticMesh>(Vertices[i], Indicies[i]));
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