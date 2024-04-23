//
// Created by MarvelLi on 2024/3/5.
//

#include "PlaneGeometry.h"



bool MechEngine::Algorithm::PlaneGeometry::IsConvexPolygon(const TArray<FVector2>& Polygon)
{
	auto crossProduct = [](const FVector2& O, const FVector2& A, const FVector2& B) {
		return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
	};
	int n = Polygon.size();
	if (n < 3) return false;

	bool sign = crossProduct(Polygon[0], Polygon[1], Polygon[2]) >= 0;
	for (int i = 1; i < n; ++i) {
		double cross = crossProduct(Polygon[i], Polygon[(i + 1) % n], Polygon[(i + 2) % n]);
		if ((cross >= 0) != sign) return false;
	}
	return true;

}