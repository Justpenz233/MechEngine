#pragma once
#include "CoreMinimal.h"

class ENGINE_API ParametricSurface : public Object
{
public:
	const bool bIsClosed;

	explicit ParametricSurface(bool bInIsClosed) : bIsClosed(bInIsClosed) {}

    virtual FVector Sample(double u, double v) const = 0;

    virtual Vector3d SampleThickness(double u, double v, double Thickness) const
    {
    	if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
        return Sample(u, v) + SampleNormal(u, v) * Thickness;
    }

    /// Return normalized vertex normal
    /// @math (Sample(u + 0.01, v) - Sample(u - 0.01, v)) X (Sample(u, v + 0.01) - Sample(u, v - 0.01)) norm
    virtual FVector SampleNormal(double u, double v) const
    {
    	Vector3d TangentU = (Sample(std::min(1., u + 0.01), v)
			- Sample(std::max(u - 0.01, 0.), v)).normalized();
    	Vector3d TangentV = (Sample(u, std::min(v + 0.01, 1.))
			- Sample(u, std::max(0., v - 0.01))).normalized();
        return TangentU.cross(TangentV).normalized();
    };

    virtual FVector SampleTangent(double u, double v) const
    {
    	Vector3d TangentU = (Sample(std::min(1., u + 0.01), v)
    		- Sample(std::max(u - 0.01, 0.), v)).normalized();
    	Vector3d TangentV = (Sample(u, std::min(v + 0.01, 1.))
    		- Sample(u, std::max(0., v - 0.01))).normalized();
    	Vector3d Tangent = (TangentU + TangentV).normalized();
        return Tangent;
    };

    // Tangent Along U direction
    virtual FVector SampleTangentU(double u, double v) const
	{
		return (Sample(std::min(1., u + 0.01), v)
			- Sample(std::max(u - 0.01, 0.), v)).normalized();
	}

	// Tangent Along V direction
	virtual FVector SampleTangentV(double u, double v) const
	{
		return (Sample(u, std::min(v + 0.01, 1.))
			- Sample(u, std::max(0., v - 0.01))).normalized();
	}

	// Project a 3d point to 2d surface
	// By default using a numeric method solve MSE problem
	virtual Vector2d Projection(const FVector& Pos) const;

	/***
	 * Find a point on the surface in given thickness, minimize the distance between the point and the given position
	 * @param Pos Given position in world space
	 * @param Thickness Thickness of the surface to sample
	 * @return UV coordinate which is closest to the point
	 */
	virtual Vector2d ProjectionThickness(const FVector& Pos, double Thickness) const;

	virtual Vector2d ProjectionWithU(const FVector& Pos, double U) const
	{
		ASSERTMSG(false, "Not Implemented");
		return {};
	}

	virtual Vector2d ProjectionWithV(const FVector& Pos, double V) const
    {
    	ASSERTMSG(false, "Not Implemented");
    	return {};
    }
};


class CylinderSurface : public ParametricSurface
{
protected:
    double Height = 1.;
    double Radius = 1.;

public:
    CylinderSurface(double InHeight = 1., double InRadius = 1.)
    : Height(InHeight), Radius(InRadius), ParametricSurface(true) {}
    double GetHegiht() const { return Height; }
	double GetRadius() const { return Radius; }
    virtual FVector Sample(double u, double v) const override
    {
	    if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
	    return {Radius * cos(u * M_PI * 2.0),
        Radius * sin(u * M_PI * 2.0), v * Height};
    }
    virtual FVector SampleThickness(double u, double v, double Thickness) const override
    {
    	if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
        return {(Radius + Thickness) * cos(u * M_PI * 2.0), (Radius + Thickness) * sin(u * M_PI * 2.0), v * Height};
    }
    virtual FVector SampleNormal(double u, double v) const override
    {
    	if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
        FVector Du = {- Radius * M_PI * 2. * sin(u * M_PI * 2.0), Radius * M_PI * 2.0 * cos(u * M_PI * 2.0), 0};
        FVector Dv = {0, 0, Height};
        return Du.cross(Dv).normalized();
    }
	// virtual Vector2d Projection(const FVector& Pos) const override
	// {
	// 	double U = atan2(Pos.y(), Pos.x()) / (2. * M_PI);
	// 	double V = Pos.z() / Height;
	// 	return {U, V};
	// }
};

class HyperbolicCylinderSurface : public ParametricSurface
{
protected:
    double Height = 1.0;
	double A = 4.0;
	double B = 2.0;

public:
    HyperbolicCylinderSurface(double _A = 4.0, double _B = 2.0, double _Hegiht = 1.0)
    : Height(_Hegiht), A(_A), B(_B), ParametricSurface(true) { }
    FVector Sample(double u, double v) const override
	{
		u = ( 0.5f - u ) * 2.f;
		v *= Height;
		return {A * sinh(u), B * cosh(u), v};
	}
	virtual FVector SampleThickness(double u, double v, double Thickness) const override
	{
		u = ( 0.5f - u ) * 2.f;
		v *= Height;
		return {A * sinh(u), B * cosh(u) + Thickness, v};
	}
	// virtual Vector2d Projection(const FVector& Pos) const override
	// {
	// 	double U = asinh(Pos.x() / A) / 2. + 0.5;
	// 	double V = Pos.z() / Height;
	// 	return {U, V};
	// }
};

class PlaneSurface : public ParametricSurface
{
protected:
	double Length = 1.;
	double Width = 1.;
public:
	PlaneSurface(double InLength = 1., double InWidth = 1.)
	: Length(InLength), Width(InWidth), ParametricSurface(false) {}

	FVector Sample(double u, double v) const override
	{
		return {u * Length, v * Width, 0};
	}
};

class ConeSurface : public ParametricSurface
{
protected:
	double Height = 1;
	double Radius = 1;
public:
    ConeSurface(double InHeight = 1., double InRadius = 1.)
    : Height(InHeight), Radius(InRadius), ParametricSurface(true) {}
    double GetHegiht() const { return Height; }
	double GetRadius() const { return Radius; }
    FVector Sample(double u, double v) const override
	{
    	if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
		v = 1. - v;
		return {Radius * cos(u * M_PI * 2.0) * v, Radius * sin(u * M_PI * 2.0) * v, (1. - v) * Height};
	}
	FVector SampleThickness(double u, double v, double Thickness) const override
	{
    	if(u < 0) u += 1.;
    	if(u > 1.) u -= 1.;
		v = 1. - v;
		return {(Radius + Thickness) * cos(u * M_PI * 2.0) * v, (Radius + Thickness) * sin(u * M_PI * 2.0) * v, (1. - v) * (Height + Thickness)};
	}
	// Vector2d Projection(const FVector& Pos) const override
	// {
 //    	Vector2d UV;
 //    	UV.y() = Pos.z() / Height;
 //    	UV.x() = atan2(Pos.y(), Pos.x()) / (2. * M_PI);
 //    	return UV;
	// }
};

class MobiusStripSurface : public ParametricSurface
{
public:
	MobiusStripSurface() :ParametricSurface(true) {}
	FVector Sample(double u, double v) const override
	{
		if(u < 0) u += 1.;
		if(u > 1.) u -= 1.;
		v = v * 2.0 - 1.0;
		return {(1 + 0.5*v*cos(u*M_PI))*cos(u*2.0*M_PI), (1 + 0.5*v*cos(u*M_PI))*sin(u*2.0*M_PI), 0.5*v*sin(u*M_PI)};
	}
};

// @see https://en.wikipedia.org/wiki/Catenoid
class CatenoidSurface : public ParametricSurface
{
private:
	double c;
public:
	CatenoidSurface(double InC = 1.): ParametricSurface(true), c(InC) {}

	FVector Sample(double u, double v) const override
	{
		if(u < 0) u += 1.;
		if(u > 1.) u -= 1.;
		u = u * 2.0 * M_PI - M_PI;
		v = v * 2.0 - 1.0;
		return {c * cosh(v/c) * cos(u), c * cosh(v/c) * sin(u), v};
	}
	FVector SampleThickness(double u, double v, double Thickness) const override
	{
		if(u < 0) u += 1.;
		if(u > 1.) u -= 1.;
		u = u * 2.0 * M_PI - M_PI;
		v = v * 2.0 - 1.0;
		FVector2 XY = {c * cosh(v/c) * cos(u), c * cosh(v/c) * sin(u)};
		XY += XY.normalized() * Thickness;
		return {XY.x(), XY.y(), v};
	}
};


class PluckeConoidSurface : public ParametricSurface
{
private:
	int n;
public:
	PluckeConoidSurface(int InN = 3): ParametricSurface(true), n(InN) {}
	FVector Sample(double u, double v) const override
	{
		if(u < 0) u += 1.;
		if(u > 1.) u -= 1.;
		u = u * 2.0 * M_PI;
		return {v*cos(u), v*sin(u), sin(n*u)};
	}
};