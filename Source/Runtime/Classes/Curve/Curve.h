//
// Created by Mavel Li on 12/9/23.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "Object/Object.h"
#include <fstream>
#include "Misc/Path.h"

class ENGINE_API Curve: public Object
{
protected:
	bool bClosed{};
    String FilePath;
    TArray<FVector> Lines;

public:
	Curve() = default;

	explicit Curve(const TArray<FVector>& InLines);

	explicit Curve(const TArray<FVector>& InLines, bool InClosed);


	FORCEINLINE int Num() const { return Lines.size(); }
    FORCEINLINE int GetPointsNum() const { return Lines.size(); }
    FORCEINLINE int GetEdgeNum() const { return bClosed ? Lines.size() : Lines.size() - 1; }
	FORCEINLINE bool IsClosed() const { return bClosed; }

	void SetCurveData(const TArray<FVector>& InLines);
	TArray<FVector> GetCurveData() const { return Lines; }

    bool ReadFromPath(const Path& InFilePath);

	virtual FVector Sample(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
		int N = Num();
        int Index = u * (N - 1);
        int NextIndex = (Index + 1 == N ? (bClosed ? 0 : N - 1) : Index + 1);
        double u0 = u * (N - 1) - Index;
        return Lines[Index] * (1. - u0) + Lines[NextIndex] * u0;
    }

	virtual FVector SampleTangent(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
        double u1 = std::clamp(u + 0.001, 0., 1.);
        double u2 = std::clamp(u - 0.001, 0., 1.);
        return (Sample(u1) - Sample(u2)).normalized();
    }

    virtual FVector SampleNormal(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
        return SampleTangent(u).cross(Sample(u)).normalized();
    }

    FORCEINLINE FVector SampleIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        return Lines[Index]; 
    }

    FORCEINLINE FVector SampleTangentIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        if(Index == Lines.size() - 1)
            return (Lines[Index] - Lines[Index - 1]).normalized();
        return (Lines[(Index + 1) % Lines.size()] - Lines[Index]).normalized();
    }

    FORCEINLINE FVector SampleNormalIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        if(Index == Lines.size() - 1)
            return ((Lines[Index] - Lines[Index - 1]).cross(Lines[Index])).normalized();
        return ((Lines[(Index + 1) % Lines.size()] - Lines[Index]).cross(Lines[Index])).normalized();
    }

    FORCEINLINE FVector& operator [] (int Index) { return Lines[Index]; }

    FORCEINLINE bool IsValid() const { return GetPointsNum() > 0; }

	/**
	 * Calculate the the minimum distance between two curves
	 * O(N*M) slow
	 * @param Other
	 * @return
	 */
	double Distance(const Curve& Other) const;

    template<class CurveType>
    double CalcSimilarity(ObjectPtr<CurveType> Others)
    {
        double Result = 0;
        for(double t = 0.; t <= 1.; t += 0.005)
        {
            Result += (Sample(t) - Others->Sample(t)).norm();
        }
        return Result;
    }

    static ObjectPtr<Curve> TrefoilKnot()
    {
        TArray<FVector> CurveData;
        for(double t = -M_PI; t <= M_PI; t += 0.01)
        {
            double x = cos(t) + 2*cos(2*t);
            double y = sin(t) - 2*sin(2*t);
            double z = 1.2*sin(3*t);
            CurveData.emplace_back(x, y, z);
        }
        return NewObject<Curve>(CurveData, true);
    }

    static ObjectPtr<Curve> PanCake()
    {
        TArray<FVector> CurveData;
        for(double t = -M_PI; t <= M_PI; t += 0.01)
        {
            double x = 2*cos(t);
            double y = 2*sin(t);
            double z = cos(2*t);
            CurveData.emplace_back(x, y, z);
        }
        return NewObject<Curve>(CurveData, true);
    }

    static ObjectPtr<Curve> TennisBallSeam()
    {
        TArray<FVector> CurveData;
        for(double t = -M_PI; t <= M_PI; t += 0.01)
        {
            double x = 2*cos(t) + cos(3*t);
            double y = 2*sin(t) - sin(3*t);
            double z = 1.2*cos(2*t);
            CurveData.emplace_back(x, y, z);
        }
        return NewObject<Curve>(CurveData, true);
    }

    static ObjectPtr<Curve> Heart()
    {
        TArray<FVector> CurveData;
        for(double t = -M_PI; t <= M_PI; t += 0.01)
        {
            double x = 16*sin(t)*sin(t)*sin(t);
            double y = 13*cos(t)-5*cos(2*t)-2*cos(3*t)-cos(4*t);
            double z = 0;
            CurveData.emplace_back(x, y, z);
        }
        return NewObject<Curve>(CurveData, true);
    }

    static ObjectPtr<Curve> Rect(double Width, double Length)
    {
        double z = 0.;
        TArray<FVector> CurveData;
        double HalfLength = Length / 2.;
        double HalfWidth = Width / 2.;
        for (double x = -HalfLength; x <= HalfLength; x += 0.01)
            CurveData.emplace_back(x, -HalfWidth, z);
        for (double y = -HalfWidth; y <= HalfWidth; y += 0.01)
            CurveData.emplace_back(HalfLength, y, z);
        for (double x = HalfLength; x >= -HalfLength; x -= 0.01)
            CurveData.emplace_back(x, HalfWidth, z);
        for (double y = HalfWidth; y >= -HalfWidth; y -= 0.01)
            CurveData.emplace_back(-HalfLength, y, z);
        return NewObject<Curve>(CurveData, true);
    }

	static ObjectPtr<Curve> MobiusStrip()
    {
    	TArray<FVector> CurveData;
    	for(double u = 0; u <= 1.; u += 0.01)
    	{
    		double x = (1 + 0.5*cos(u*M_PI)) * cos(u*2.0*M_PI);
    		double y = (1 + 0.5*cos(u*M_PI)) * sin(u*2.0*M_PI);
    		double z = 0.5*sin(u*M_PI);
    		CurveData.emplace_back(x, y, z);
    	}
    	return NewObject<Curve>(CurveData, true);
    }

	static ObjectPtr<Curve> Circle(double Radius = 1.)
    {
    	TArray<FVector> CurveData;
    	for(double u = 0; u <= 1.; u += 0.01)
    	{
    		double x = Radius * sin(u*2.0*M_PI);
    		double y = Radius * cos(u*2.0*M_PI);
    		double z = 0;
    		CurveData.emplace_back(x, y, z);
    	}
    	return NewObject<Curve>(CurveData, true);
    }

	static ObjectPtr<Curve> Ellipse(double a, double b)
    {
    	TArray<FVector> CurveData;
    	for(double u = 0; u <= 1.; u += 0.01)
    	{
    		double x = a * cos(u*2.0*M_PI);
    		double y = b * sin(u*2.0*M_PI);
    		double z = 0;
    		CurveData.emplace_back(x, y, z);
    	}
    	return NewObject<Curve>(CurveData, true);
    }
};
