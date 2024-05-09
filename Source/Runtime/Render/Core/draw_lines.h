//
// Created by MarvelLi on 2024/4/30.
//

#pragma once

#include <luisa/luisa-compute.h>
#include "view_data.h"
#include "Misc/Platform.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    /**
     * Calulate the distance from a point to a segment in 2D space.
     * @param p point in 2D space
     * @param x start point of the segment
     * @param y end point of the segment
     * @return the distance from the point to the segment
     */
    FORCEINLINE Float distance_to_segment(const Float2& p, const Float2& x, const Float2& y)
    {
        auto l = y - x;
        auto t = dot(p - x, l) / dot(l, l);
        auto dis = length(p - (x + t * l));
        $if (t < 0.f) {dis = length(p - x);};
        $if (t > 1.f) {dis = length(p - y);};
        return dis;
    }

    /**
     * Given the viewport coordinate, test if the pixel is in the segments x -> y with given thickness.
     * @param p pixel in viewport space
     * @param x start point of the line in viewport space
     * @param y end point of the line in viewport space
     * @param thickness thickness of the line in pixel
     */
    FORCEINLINE Bool on_the_lines(const Float2& p, const Float2& x, const Float2& y, const Float& thickness)
    {
        return distance_to_segment(p, x, y) <= thickness;
    }

    /**
    * Given the viewport coordinate, and world space triangle, test if the pixel is in the triangle wire frame with given thickness.
    * @see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
    * @see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
    * @param view current view
    * @param p pixel in viewport space
    * @param v0 vertex 0 of the triangle in NDC space
    * @param v1 vertex 1 of the triangle in NDC space
    * @param v2 vertex 2 of the triangle in NDC space
    * @param thickness thickness of the line in pixel
    */
    FORCEINLINE Bool IsWireFrame(Var<view_data> view, const Float2& p, const Float4& v0, const Float4& v1, const Float4& v2, const Float& thickness)
    {
        auto v0_coord = view->ndc_to_screen(v0);
        auto v1_coord = view->ndc_to_screen(v1);
        auto v2_coord = view->ndc_to_screen(v2);
        return on_the_lines(p, v0_coord, v1_coord, thickness) |
               on_the_lines(p, v1_coord, v2_coord, thickness) |
               on_the_lines(p, v2_coord, v0_coord, thickness);
    }
}
