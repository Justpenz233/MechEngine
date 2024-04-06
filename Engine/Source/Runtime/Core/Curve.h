//
// Created by Mavel Li on 12/9/23.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "Object/Object.h"
#include <cmath>
#include <fstream>
#include <limits>

class Curve: public Object
{
protected:
    String FilePath;
    int Num;
    TArray<FVector> Lines;

public:
    bool bClosed;

    Curve(String InFilePath = "") : FilePath(InFilePath) 
    {
        if(InFilePath != "")
        {
            ReadFromPath(InFilePath);
        }
    }

    Curve(const TArray<FVector>& InLines, bool InClosed): bClosed(InClosed) { SetCurveData(InLines); }
    
    int GetPointsNum() {return Lines.size(); }
    int GetEdgeNum() {return Lines.size(); }

    bool ReadFromPath(String InFilePath = "") 
    {
        if(InFilePath == "") InFilePath = FilePath;
        std::ifstream File(InFilePath);
        if(!File.good()){
            LOG_ERROR("Open curve file: {0} failed", InFilePath);
            return false;
        }
        else
        {
            File >> Num;
            for(int i = 1;i <= Num;i ++)
            {
                FVector NewPoint;
                File >> NewPoint[0] >> NewPoint[1] >> NewPoint[2];
                if(i != Num && File.eof())
                {
                    LOG_ERROR("Curve file {0} format error, reach EOF early.", InFilePath);
                    Lines.clear();
                    return false;
                }
                Lines.push_back(NewPoint);
            }
            File.close();
        }
        return true;
    }

    inline FVector Sample(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
        int Index = u * (Num - 1);
        int NextIndex = std::min(Index + 1, Num - 1);
        double u0 = u * (Num - 1) - Index;
        return Lines[Index] * (1. - u0) + Lines[NextIndex] * u0;
    }

    FVector SampleTangent(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
        double u1 = std::clamp(u + 0.001, 0., 1.);
        double u2 = std::clamp(u - 0.001, 0., 1.);
        return (Sample(u1) - Sample(u2)).normalized();
    }

    FVector SampleNormal(double u) const
    {
        ASSERTMSG(u <= 1., "Sample U: {0} out of bounds", u);
        return SampleTangent(u).cross(Sample(u)).normalized();
    }

    inline FVector SampleIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        return Lines[Index]; 
    }

    FVector SampleTangentIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        if(Index == Lines.size() - 1)
            return (Lines[Index] - Lines[Index - 1]).normalized();
        return (Lines[(Index + 1) % Lines.size()] - Lines[Index]).normalized();
    }

    FVector SampleNormalIndex(int Index) const
    {
        ASSERTMSG(Index < Lines.size(), "Sample index out of bounds");
        if(Index == Lines.size() - 1)
            return ((Lines[Index] - Lines[Index - 1]).cross(Lines[Index])).normalized();
        return ((Lines[(Index + 1) % Lines.size()] - Lines[Index]).cross(Lines[Index])).normalized();
    }

    void SetCurveData(const TArray<FVector>& InLines)
    {
	    Lines = InLines;
    	Num = Lines.size();
    }

    const TArray<FVector>& GetData(){ return Lines; }

    FVector& operator [] (int Index) { return Lines[Index]; }

    bool IsValid() { return GetPointsNum() > 0; }

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
