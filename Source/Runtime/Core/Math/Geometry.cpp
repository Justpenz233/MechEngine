//
// Created by MarvelLi on 2024/4/22.
//

#include "Geometry.h"
#include "Mesh/StaticMesh.h"
#include <igl/copyleft/cgal/convex_hull.h>
namespace MechEngine::Math
{


std::tuple<double, FVector> PointSegmentDistance(const FVector& P, const FVector& A0, const FVector& A1)
{
	if((A0 - A1).isZero())
	{
		return {(P - A0).norm(), A0};
	}
	FVector P2;
	// http://stackoverflow.com/a/1501725/148668
	const auto sqr_len = (A0 - A1).squaredNorm();
	assert(sqr_len != 0);
	const auto & V = A0;
	const auto & W = A1;
	const auto t = (P-V).dot(W-V)/sqr_len;
	if(t<0)
		P2 = V;
	else if(t>1)
	{
		P2 = W;
	}else
	{
		P2 = V  + t*(W-V);
	}
	double d = (P-P2).norm();
	return {d, P2};
}

std::tuple<double, FVector, FVector, bool> SegmentSegmentDistance(
	const FVector& A0, const FVector& A1,
	const FVector& B0, const FVector& B1)
{
	if ((A0 - A1).isZero())
	{
		auto Result = PointSegmentDistance(A1, B0, B1);
		return { std::get<0>(Result), A1, std::get<1>(Result), false };
	}
	if ((B0 - B1).isZero())
	{
		auto Result = PointSegmentDistance(B1, A0, A1);
		return { std::get<0>(Result), std::get<1>(Result), B1, false };
	}

	FVector u = A1 - A0;
	FVector v = B1 - B0;
	FVector w = A0 - B0;

	const auto a = u.dot(u); // always >= 0
	const auto b = u.dot(v);
	const auto c = v.dot(v); // always >= 0
	const auto d = u.dot(w);
	const auto e = v.dot(w);
	const auto D = a * c - b * b; // always >= 0
	assert(D >= 0);
	auto sc = D, sN = D, sD = D; // sc = sN / sD, default sD = D >= 0
	auto tc = D, tN = D, tD = D; // tc = tN / tD, default tD = D >= 0

	bool parallel = false;
	// compute the line parameters of the two closest points
	if (D == 0)
	{
		// the lines are almost parallel
		parallel = true;
		sN = 0.0; // force using source point on segment S1
		sD = 1.0; // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else
	{
		// get the closest points on the infinite lines
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0.0)
		{
			// sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD)
		{
			// sc > 1  => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0)
	{
		// tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
		{
			sN = 0.0;
		}
		else if (-d > a)
		{
			sN = sD;
		}
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)
	{
		// tc > 1  => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
		{
			sN = 0;
		}
		else if ((-d + b) > a)
		{
			sN = sD;
		}
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) == 0 ? 0.0 : sN / sD);
	tc = (abs(tN) == 0 ? 0.0 : tN / tD);

	// get the difference of the two closest points
	FVector P1 = A0 + sc * (A1 - A0);
	FVector P2 = B0 + sc * (B1 - B0);
	FVector dP = w + (sc * u) - (tc * v); // =  S1(sc) - S2(tc)
	assert(dP == (P1 - P2));

	double dst = dP.norm(); // return the closest distance
	return { dst, P1, P2, parallel };
}

ObjectPtr<StaticMesh> ConvexHull(const ObjectPtr<StaticMesh>& Mesh)
{
	MatrixX3d Vertices = Mesh->GetVertices();
	MatrixX3i Triangles;
	igl::copyleft::cgal::convex_hull(Vertices, Triangles);
	return NewObject<::StaticMesh>(std::move(Vertices), std::move(Triangles));
}

} // namespace MechEngine::Math