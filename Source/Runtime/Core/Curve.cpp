//
// Created by MarvelLi on 2024/4/22.
//
#include "Curve.h"
#include "Math/Geometry.h"


Curve::Curve(const TArray<FVector>& InLines)
{
	SetCurveData(InLines);
}

Curve::Curve(const TArray<FVector>& InLines, bool InClosed)
{
	SetCurveData(InLines);
	bClosed = InClosed;
}

void Curve::SetCurveData(const TArray<FVector>& InLines)
{
	Lines = InLines;
	bClosed = ((Lines[0] - Lines[Lines.size() - 1]).norm() < 1e-6);
}

bool Curve::ReadFromPath(const Path& InFilePath)
{
	if(!InFilePath.Existing())
	{
		LOG_ERROR("Curve file: {0} not exist", InFilePath.string());
		return false;
	}
	std::ifstream File(InFilePath);
	if(!File.good()){
		LOG_ERROR("Open curve file: {0} failed", InFilePath.string());
		return false;
	}
	else
	{
		int Num;
		File >> Num;
		for(int i = 1;i <= Num;i ++)
		{
			FVector NewPoint;
			File >> NewPoint[0] >> NewPoint[1] >> NewPoint[2];
			if(i != Num && File.eof())
			{
				LOG_ERROR("Curve file {0} format error, reach EOF early.", InFilePath.string());
				Lines.clear();
				return false;
			}
			Lines.push_back(NewPoint);
		}
		File.close();
	}
	return true;
}

double Curve::Distance(const Curve& Other) const
{
	double D = std::numeric_limits<double>::max();
	for (int i = 0; i < Num() - 1; i++)
	{
		FVector P0 = SampleIndex(i);
		FVector P1 = SampleIndex(i + 1);
		for (int j = 0; j < Other.Num() - 1; j++)
		{
			FVector Q0 = Other.SampleIndex(j);
			FVector Q1 = Other.SampleIndex(j + 1);
			auto Result = Math::SegmentSegmentDistance(P0, P1, Q0, Q1);
			D = std::min(D, std::get<0>(Result));
		}
	}
	return D;
}
