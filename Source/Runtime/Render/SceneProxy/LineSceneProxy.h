//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;
    struct lines_data
    {
        // Vertex buffer id in bindless array
        uint vertex_id = ~0u;

        // Transform id
        uint transform_id = ~0u;

        // thickness of the curve, in pixel
        float thickness = -1.f;

        // color of the curve
        float3 color;
    };

    struct point_data
    {
        // Vertex buffer id in bindless array
        uint vertex_id = ~0u;

        // Transform id
        uint transform_id = ~0u;

        // size of the point, in pixel
        float size = -1.f;

        // color of the point
        float3 color;
    };
}

LUISA_STRUCT(MechEngine::Rendering::lines_data, vertex_id, transform_id, thickness, color) {};
LUISA_STRUCT(MechEngine::Rendering::point_data, vertex_id, transform_id, size, color) {};


namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    class LineSceneProxy : public SceneProxy
    {
    public:
        explicit LineSceneProxy(RayTracingScene& InScene) noexcept;

        virtual void UploadDirtyData(luisa::compute::Stream& stream) override {}

        // void AddLines();
        //
        // void RemoveLines();
        //
        // void AddBox();
        //
        // void RemoveBox();
        //
        // void AddPoint();
        //
        // void RemovePoint();
        //
        // /***
        //  * Update thickness of the primitive
        //  */
        // void UpdateThickness();

    protected:
        static constexpr uint32_t MaxCurveCount = 1024;
        BufferView<lines_data> lines_data_buffer;
    };
}