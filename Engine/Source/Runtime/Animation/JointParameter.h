//
// Created by MarvelLi on 2023/12/19.
//

#pragma once
#include "CoreMinimal.h"
#include "Math/FTransform.h"

#include <Eigen/Eigen>

enum EDOF3D {
	FreeNone = 0,
	FreeX = 0b001,
	FreeY = 0b010,
	FreeZ = 0b100,
	FreeXY = 0b011,
	FreeXZ = 0b101,
	FreeYZ = 0b110,
	FreeXYZ = 0b111
};

struct JointLocationParamter
{
private:
	EDOF3D DOF = FreeNone;
	FVector Location = FVector::Zero();

public:
	//Construct from DOF
	JointLocationParamter(EDOF3D InDOF):DOF(InDOF){}
	operator FVector&();
	operator FVector();
	Matrix4d MakeMatrix4d();
	inline int ParameterNum() const;

	/// \brief Will fetch parameter from vector, the vertor will be shorten
	/// \param Parameter Parameter vector
	inline void ReadParameter(VectorXd &Parameter);
	VectorXd GetParameter() const;
	friend class JointParameter;
};


struct JointRotationParamter
{
private:
	EDOF3D DOF = FreeNone;
	FVector RotationEuler = FVector::Zero();

public:
	//Construct from DOF
	JointRotationParamter(EDOF3D InDOF):DOF(InDOF){}
	operator FVector&();
	operator FVector();
	operator FQuat();
	Matrix3d MakeMatrix3d();
	Matrix4d MakeMatrix4d();
	inline int ParameterNum() const;

	/// \brief Will fetch parameter from vector, the vertor will be shorten
	/// \param Parameter Parameter vector
	inline void ReadParameter(VectorXd &Parameter);

	VectorXd GetParameter() const;
	friend class JointParameter;
};

/// \brief Joint driven paramter. Used for dive a joint by IK method or straightly addtive. Oreder is rotation then location
class JointParameter
{
private:
	JointRotationParamter RotationParam;
	JointLocationParamter LocationParam;

public:
	///Construct form DOF
	JointParameter(EDOF3D InRoationDOF, EDOF3D InTranslationDOF);

	operator FTransform();
	explicit operator Matrix4d();
	int ParameterNum() const;

	/// \brief Will fetch parameter from vector, the vertor will be shorten, first read location then rotation
	/// \param Parameter Parameter vector
	void ReadParameter(VectorXd &Parameter);

	FVector GetLocation() const;
	FVector GetRotationEuler() const;

	//Get Parameters
	VectorXd GetParameter() const;
};