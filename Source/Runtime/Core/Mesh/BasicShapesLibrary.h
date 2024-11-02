//
// Created by Siqi Li on 6/25/2023.
//

#pragma once


#include "Curve/Curve.h"
#include "StaticMesh.h"

class ENGINE_API BasicShapesLibrary {

public:
    // Create a Curve Mesh
    static ObjectPtr<StaticMesh> GenerateCurveMesh(const TArray<FVector>& CurveData, double Radius, bool bClosed, bool bUseCubiod = false, int RingSample = 32, int SampleNum = 64);
    static ObjectPtr<StaticMesh> GenerateCurveMesh(ObjectPtr<Curve> CurveData, double Radius, bool bClosed, int RingSample = 32, int SampleNum = 64);
	static ObjectPtr<StaticMesh> GenerateCurveMeshCubiod(ObjectPtr<Curve> CurveData, double Radius, bool bClosed, int SampleNum = 64);


	/**
	 * Generate a mesh by extruding a contour along a curve, the curve will be force closed
	 * @param CurveData Curve data
	 * @param ContourSample Sample function for contour, return a 2D point by a parameter. In the 2D plane, the Y is Normal of the curve, X is the binormal of the curve
	 * @param ControuSample Sample number of contour
	 * @param CurveSample Sample number of curve
	 * @return Mesh generated mesh asset
	 */
	static ObjectPtr<StaticMesh> GenerateExtrudeMesh(const TArray<FVector>& CurveData, const TFunction<FVector2(double)>& ContourSample, int ControuSample = 32, int CurveSample = 64);


    static ObjectPtr<StaticMesh> GenerateSphere(double Radius, int Sample = 32);
	static ObjectPtr<StaticMesh> GenerateHemisphere(double Radius, bool bTopHalf = true, int Sample = 32);

    // Central at orgin, samples for circle, Height in z direction
    static ObjectPtr<StaticMesh> GenerateCylinder(double length, double radius, int Samples = 32);
    static ObjectPtr<StaticMesh> GenerateCylinder(const Vector3d& A, Vector3d B, double Radius, int Samples = 32);

	/**
	 * Generate a capsule
	 * @param Radius Radius of the capsule
	 * @param Height Height of the capsule body, so the Box height is Height + 2 * Radius
	 * @return Mesh
	 */
	static ObjectPtr<StaticMesh> GenerateCapsule(double Radius, double Height, int Sample = 32, int RingSample = 32);

	/**
	 * Generate a capsule with one hat
	 * @param Radius Radius of the capsule
	 * @param Height Height of the capsule body, so the Box height is Height + Radius
	 */
	static ObjectPtr<StaticMesh> GenerateOneHatCapsule(double Radius, double Height, bool bTopHat = true, int Sample = 32, int RingSample = 32);

	static ObjectPtr<StaticMesh> GenerateHollowCylinder(const Vector3d& A, const Vector3d& B, double OuterRadius, double InnerRadius, int Samples = 32);
    static ObjectPtr<StaticMesh> GenerateHollowCylinder(double OuterRadius, double InnerRadius, double Height, int Sample = 32);

	// Generate a cuboid with different size in x, y, z direction
    static ObjectPtr<StaticMesh> GenerateCuboid(Vector3d size);
	static ObjectPtr<StaticMesh> GenerateCuboid(Vector3d A, Vector3d B, Vector2d Size);

	// Generate a cuboid with same size in x, y, z direction
	static ObjectPtr<StaticMesh> GenerateCuboid(double size);


	static ObjectPtr<StaticMesh> GeneratePlane(const FVector2& Size = FVector2::Ones());

	/**
	 * Generate a fan shape, with a height in Z direction, as (sin(theta), cos(theta), z)
	 * @param Radius Radius of the fan
	 * @param Height Height of the fan
	 * @param StartAngle Start angle of the fan
	 * @param EndAngle End angle of the fan
	 * @param Sample Sample number of radial direction
	 * @return A static mesh
	 */
	static ObjectPtr<StaticMesh> GenerateFan(double Radius, double Height, double StartAngle, double EndAngle, int Sample = 32);


    /**
     * Triangular a surface by providing a sample function.
     * @param NumU How many samples in U direction
     * @param NumV How many samples in V direction
     * @param SampleFunc Sample function, return a 3D point by a 2D parameter
     * @param ReverseNormal If the normal is head to inside, this is useful when creating a inner surface
     * @param ClosedSurface If the surface is a closed
     * @return Surface mesh
     */
    static ObjectPtr<StaticMesh> TriangularSurface(int NumU, int NumV, std::function<FVector(double, double)> SampleFunc, bool ReverseNormal, bool ClosedSurface = false);

    /**
     * Generate a cone mesh
     * @param Radius Radius of the cone
     * @param Height Height of the cone
     * @param SealBottom If the bottom of the cone is sealed
     * @param SampleRing Sample number of the ring
     * @param SampleHeight Sample number of the height
     * @return Cone mesh
     */
    static ObjectPtr<StaticMesh> GenerateCone(double Radius, double Height, bool SealBottom, int SampleRing = 32, int SampleHeight = 32);
};