// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2017 Zhongshi Jiang <jiangzs@nyu.edu>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IGL_EXACT_GEODESIC_H
#define IGL_EXACT_GEODESIC_H

#include "igl_inline.h"
#include <Eigen/Core>

namespace igl 
{
  /// Exact geodesic algorithm for triangular mesh with the implementation from https://code.google.com/archive/p/geodesic/, 
  /// and the algorithm first described by Mitchell, Mount and Papadimitriou in 1987
  ///
  /// @param[in] V  #V by 3 list of 3D vertex positions
  /// @param[in] F  #F by 3 list of mesh faces
  /// @param[in] VS #VS by 1 vector specifying indices of source vertices
  /// @param[in] FS #FS by 1 vector specifying indices of source faces
  /// @param[in] VT #VT by 1 vector specifying indices of target vertices
  /// @param[in] FT #FT by 1 vector specifying indices of target faces
  /// @param[out] D  #VT+#FT by 1 vector of geodesic distances of each target w.r.t. the nearest one in the source set
  ///
  /// \note specifying a face as target/source means its center. 
  ///
    template <
    typename DerivedV,
    typename DerivedF,
    typename DerivedVS,
    typename DerivedFS,
    typename DerivedVT,
    typename DerivedFT,
    typename DerivedD>
    IGL_INLINE void exact_geodesic(
      const Eigen::MatrixBase<DerivedV> &V,
      const Eigen::MatrixBase<DerivedF> &F,
      const Eigen::MatrixBase<DerivedVS> &VS,
      const Eigen::MatrixBase<DerivedFS> &FS,
      const Eigen::MatrixBase<DerivedVT> &VT,
      const Eigen::MatrixBase<DerivedFT> &FT,
      Eigen::PlainObjectBase<DerivedD> &D);




	template <
	typename DerivedV,
	typename DerivedF>
	IGL_INLINE Eigen::MatrixX3d exact_geodesic_path(
	  const Eigen::MatrixBase<DerivedV> &V,
	  const Eigen::MatrixBase<DerivedF> &F,
	  const int &VS,
	  const int &VT
	  );


	/**
	 * Find the triangle path from source to target
	 */
	template <
	typename DerivedV,
	typename DerivedF>
	IGL_INLINE Eigen::VectorXi exact_geodesic_path_tri(
	  const Eigen::MatrixBase<DerivedV> &V,
	  const Eigen::MatrixBase<DerivedF> &F,
	  const int &TriSt,
	  const int &TriDes
	  );

	template <
	typename DerivedV,
	typename DerivedF,
	typename DerivedVS,
   typename DerivedVT>
	IGL_INLINE std::vector<Eigen::Vector3d> exact_geodesic_path(
	  const Eigen::MatrixBase<DerivedV> &V,
	  const Eigen::MatrixBase<DerivedF> &F,
	  const Eigen::MatrixBase<DerivedVS> &VS,
	  const Eigen::MatrixBase<DerivedVT> &VT,
	  const int& VSIndex,
	  const int& VTIndex
	  );
	}

#ifndef IGL_STATIC_LIBRARY
#  include "exact_geodesic.cpp"
#endif

#endif
