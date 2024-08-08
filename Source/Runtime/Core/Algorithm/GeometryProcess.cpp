//
// Created by MarvelLi on 2024/1/30.
//

#include "GeometryProcess.h"
#include "Mesh/StaticMesh.h"
#include "igl/swept_volume.h"
#include "Math/FTransform.h"
#include "Mesh/MeshBoolean.h"
#include "igl/vertex_components.h"

#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <igl/boundary_loop.h>

#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

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


	ObjectPtr<StaticMesh> SweptVolume(const ObjectPtr<StaticMesh>& Profile, const TArray<FTransform>& Path, int Steps, int GridSize)
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
		}, Steps, GridSize, 0, RV, RF);
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


	struct SurfaceProjectFunctor
	{
		using Scalar = double;
		enum {
			InputsAtCompileTime = 2, // UV
			ValuesAtCompileTime = 3  // distance to [x, y, z]
		};
		typedef Eigen::Matrix<double,Eigen::Dynamic,1> InputType;
		typedef Eigen::Matrix<double,Eigen::Dynamic,1> ValueType;
		typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> JacobianType;
		typedef std::function<FVector(double u, double v)> SurfaceSampleFunc;

		SurfaceSampleFunc SampleFunc;
		FVector Target;
		const int m_inputs, m_values;

		SurfaceProjectFunctor(SurfaceSampleFunc&& InSurface, const FVector& InTarget) : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime),
			 Target(InTarget), SampleFunc(InSurface) {}

		int inputs() const { return m_inputs; }
		int values() const { return m_values; }

		int operator()(const Eigen::VectorXd &UV, Eigen::VectorXd &DisXYZ) const
		{
			double U = UV(0);
			double V = UV(1);

			double Penalty = 0.;
			if(V < 0. || V > 1.) Penalty += 1000.;
			if(U < 0. || U > 1.) Penalty += 1000.;

			V = std::clamp(V, 0., 1.);
			U = std::clamp(U, 0., 1.);
			Vector3d Pos = SampleFunc(U, V);

			DisXYZ.resize(3);
			DisXYZ(0) = std::pow(Pos.x() - Target.x(), 2) + Penalty;
			DisXYZ(1) = std::pow(Pos.y() - Target.y(), 2) + Penalty;
			DisXYZ(2) = std::pow(Pos.z() - Target.z(), 2) + Penalty;
			return 0;
		}
	};

	FVector2 Projection(const FVector& Pos, const TFunction<FVector(const FVector2&)>& SampleFunction)
	{
		SurfaceProjectFunctor fucnctor([&](double u, double v) {
			return SampleFunction(FVector2(u, v));
		}, Pos);
		double BestEnergy = 1e6;
		Vector2d Best;
		for (double u = 0.; u <= 1.; u += 0.1)
		{
			for (double v = 0.; v <= 1.; v += 0.1)
			{
				VectorXd UV(2);
				UV << u, v;
				Eigen::NumericalDiff<SurfaceProjectFunctor>									   numDiff(fucnctor);
				Eigen::LevenbergMarquardt<Eigen::NumericalDiff<SurfaceProjectFunctor>, double> lm(numDiff);
				int																			   t = lm.minimize(UV);

				double Energy = (SampleFunction(FVector2(u, v)) - Pos).norm();
				if (Energy < BestEnergy)
				{
					BestEnergy = Energy;
					Best = { u, v };
				}
				Energy = (SampleFunction(UV) - Pos).norm();
				if (Energy < BestEnergy)
				{
					BestEnergy = Energy;
					Best = { UV(0), UV(1) };
				}
			}
		}
		return Best;
	}

	FVector2 Projection(const FVector& Pos, const TFunction<FVector(const FVector2&)>& SampleFunction, const FVector2& InitialGuess)
	{
		SurfaceProjectFunctor fucnctor([&](double u, double v) {
			return SampleFunction(FVector2(u, v));
		}, Pos);
		VectorXd UV(2);
		UV << InitialGuess.x(), InitialGuess.y();
		Eigen::NumericalDiff<SurfaceProjectFunctor>									   numDiff(fucnctor);
		Eigen::LevenbergMarquardt<Eigen::NumericalDiff<SurfaceProjectFunctor>, double> lm(numDiff);
		int																			   t = lm.minimize(UV);
		return {UV[0], UV[1]};
	}
	ObjectPtr<StaticMesh> SolidifyMesh(const ObjectPtr<StaticMesh>& Mesh, double Thickness)
	{
		std::vector<std::vector<int>> Boundarys;
		igl::boundary_loop(Mesh->triM, Boundarys);
		auto Inner = NewObject<StaticMesh>(*Mesh);
		Inner->OffsetVertex(-Thickness);
		Inner->ReverseNormal();
		auto Result = MeshBoolean::MeshConnect(Mesh, Inner);
		// No boundary
		if (Boundarys.empty())
			return Result;

		// With boundary need to seal the boundary
		int		  IndexBias = Mesh->GetVertexNum();
		MatrixX3i TriM = Result->GetTriangles();
		// Seal the boundary
		for (const auto& Bound : Boundarys)
		{
			MatrixX3i NewTri(Bound.size() * 2, 3);
			for (int j = 0; j < Bound.size(); j++)
			{
				int		 Next = (j + 1) % Bound.size();
				Vector3i Tri;
				Tri << Bound[j], Bound[Next] + IndexBias, Bound[Next];
				NewTri.row(j * 2) = Tri;
				Tri = { Bound[j], Bound[j] + IndexBias, Bound[Next] + IndexBias };
				NewTri.row(j * 2 + 1) = Tri;
			}
			TriM.conservativeResize(TriM.rows() + NewTri.rows(), 3);
			TriM.bottomRows(NewTri.rows()) = NewTri;
		}
		Result->SetGeometry(TriM);
		return Result;
	}
	ObjectPtr<StaticMesh> SolidifyMeshEven(const ObjectPtr<StaticMesh>& Mesh, double Thickness)
	{
		std::vector<std::vector<int>> Boundarys;
		igl::boundary_loop(Mesh->triM, Boundarys);
		auto Outter = NewObject<StaticMesh>(*Mesh);
		Outter->OffsetVertex(0.5 * Thickness);
		auto Inner = NewObject<StaticMesh>(*Mesh);
		Inner->OffsetVertex(-0.5 * Thickness);
		Inner->ReverseNormal();
		auto Result = MeshBoolean::MeshConnect(Outter, Inner);
		// No boundary
		if (Boundarys.empty())
			return Result;

		// With boundary need to seal the boundary
		int		  IndexBias = Mesh->GetVertexNum();
		MatrixX3i TriM = Result->GetTriangles();
		// Seal the boundary
		for (const auto& Bound : Boundarys)
		{
			MatrixX3i NewTri(Bound.size() * 2, 3);
			for (int j = 0; j < Bound.size(); j++)
			{
				int		 Next = (j + 1) % Bound.size();
				Vector3i Tri;
				Tri << Bound[j], Bound[Next] + IndexBias, Bound[Next];
				NewTri.row(j * 2) = Tri;
				Tri = { Bound[j], Bound[j] + IndexBias, Bound[Next] + IndexBias };
				NewTri.row(j * 2 + 1) = Tri;
			}
			TriM.conservativeResize(TriM.rows() + NewTri.rows(), 3);
			TriM.bottomRows(NewTri.rows()) = NewTri;
		}
		Result->SetGeometry(TriM);
		return Result;
	}

	} // namespace MechEngine::Algorithm::GeometryProcess