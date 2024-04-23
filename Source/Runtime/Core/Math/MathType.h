//
// Created by MarvelLi on 2024/3/25.
//

#pragma once

/************************************************************
 *
 * This file responsible for math type forward declaration.
 *
 ************************************************************/


#include "Eigen/Eigen"

class FTransform;
namespace MechEngine::Math
{
template <class T>
class TBox;

using FBox = TBox<double>;
using FBox3f = TBox<float>;
using FBox3d = TBox<double>;

using Eigen::Quaterniond;
using Eigen::Vector2d;
using Eigen::Vector2f;
using Eigen::Vector2i;
using Eigen::Vector3d;
using Eigen::Vector3i;
using Eigen::Vector3;
using Eigen::Vector4d;
using Eigen::VectorXd;
using Eigen::RowVector2i;
using Eigen::RowVector3d;
using Eigen::RowVector3i;
using Eigen::Matrix3d;
using Eigen::Matrix4d;
using Eigen::Matrix4f;
using Eigen::MatrixXd;
using Eigen::MatrixXi;
using Eigen::MatrixX3d;
using Eigen::MatrixX3i;
using Eigen::AngleAxisd;
using Eigen::Affine3d;

using FVector = Eigen::Vector3d;
using FVector2 = Eigen::Vector2d;
using FVector4 = Eigen::Vector4d;

using FQuat = Eigen::Quaterniond;
using FEuler = Eigen::Vector3d;

using FMatrix = Eigen::Matrix3d;
using FMatrix4 = Eigen::Matrix4d;

using FColor = Eigen::Vector3d;
using FLinearColor = Eigen::Vector3d;
}

using namespace MechEngine::Math;

#define RGB(R,G,B) FVector(R/255.,G/255.,B/255.)

#ifndef M_PI
#define M_PI			3.1415926535897932384626433832795
#define M_E        2.71828182845904523536   // e
#define M_LOG2E    1.44269504088896340736   // log2(e)
#define M_LOG10E   0.434294481903251827651  // log10(e)
#define M_LN2      0.693147180559945309417  // ln(2)
#define M_LN10     2.30258509299404568402   // ln(10)
#define M_PI       3.14159265358979323846   // pi
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.785398163397448309616  // pi/4
#define M_1_PI     0.318309886183790671538  // 1/pi
#define M_2_PI     0.636619772367581343076  // 2/pi
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif

#define MATH_2_PI 6.28318530717958647693