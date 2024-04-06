//
// Created by MarvelLi on 2024/1/19.
//

#include "CurveDistance.h"


double MechEngine::Algorithm::DiscreteFrechetDistance(const TArray<FVector>& CurveA, const TArray<FVector>& CurveB)
{
	ASSERTMSG(!CurveA.empty() && !CurveB.empty(), "CurveA or CurveB is empty");
	auto Dist = [](const FVector& A, const FVector& B) { return (A - B).norm(); };
	const int m = CurveA.size();
	const int n = CurveB.size();
	TArray<TArray<double>> C(m, TArray<double>(n));

	C[0][0] = Dist(CurveA[0], CurveB[0]);
	for (int i = 1; i < m; i++)
		C[i][0] = std::max( Dist(CurveA[i], CurveB[0]), C[i - 1][0]);
	for (int j = 1; j < n; j++)
		C[0][j] = std::max( Dist(CurveA[0], CurveB[j]), C[0][j - 1]);
	for (int i = 1; i < m; i++)
	{
		for (int j = 1; j < n; j++)
		{
			C[i][j] = std::min({C[i - 1][j], C[i][j - 1], C[i - 1][j - 1]});
			C[i][j] = std::max(C[i][j], Dist(CurveA[i], CurveB[j]));
		}
	}
	return C[m - 1][n - 1];
}

double MechEngine::Algorithm::DTWDistance(const TArray<FVector>& CurveA, const TArray<FVector>& CurveB)
{
	auto Dist = [](const FVector& A, const FVector& B) { return (A - B).norm(); };
	const int m = CurveA.size();
	const int n = CurveB.size();
	TArray<TArray<double>> C(m, TArray<double>(n));

	C[0][0] = Dist(CurveA[0], CurveB[0]);
	for (int i = 1; i < m; i++)
		C[i][0] = Dist(CurveA[i], CurveB[0]) + C[i - 1][0];
	for (int j = 1; j < n; j++)
		C[0][j] = Dist(CurveA[0], CurveB[j]) + C[0][j - 1];

	for (int i = 1; i < m; i++)
	{
		for (int j = 1; j < n; j++)
		{
			C[i][j] = std::min({C[i - 1][j], C[i][j - 1], C[i - 1][j - 1]});
			C[i][j] += Dist(CurveA[i], CurveB[j]);
		}
	}
	return C[m - 1][n - 1];
}