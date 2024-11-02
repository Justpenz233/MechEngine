//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"

namespace MechEngine::Rendering
{
    struct view;
}

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;
    struct lines_data
    {
        // Vertex buffer id in bindless array
    	float3 world_start;

    	float3 world_end;

        // thickness of the curve, in pixel
        float thickness = -1.f;

        // color of the curve
        float3 color;
    };

    struct point_data
    {
    	// position in world
    	float3 world_position;

        // size of the point, in pixel
        float radius = -1.f;

        // color of the point
        float3 color;
    };
}

LUISA_STRUCT(MechEngine::Rendering::lines_data, world_start, world_end, thickness, color) {};
LUISA_STRUCT(MechEngine::Rendering::point_data, world_position, radius, color) {};


namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    class ENGINE_API LineSceneProxy : public SceneProxy
    {
    public:
        explicit LineSceneProxy(GpuScene& InScene) noexcept;

        virtual void UploadDirtyData(Stream& stream) override;

		uint AddLine(float3 WorldStart, float3 WorldEnd, float Thickness, float3 Color);
        //
        // void RemoveLines();
        //
        // void AddBox();
        //
        // void RemoveBox();
        //

        uint AddPoint(float3 WorldPosition, float Radius, float3 color);
        //
        // void RemovePoint();
        //
        // /***
        //  * Update thickness of the primitive
        //  */
        // void UpdateThickness();

    public:
        virtual void CompileShader() override;

        virtual void PostRenderPass(Stream& stream) override;

    protected:
        static constexpr uint32_t MaxLinesCount = 8192; // 8192 * 10 * 4 = 327680 bytes = 320 KB
        static constexpr uint32_t MaxPointCount = 8192; // 8192 * 7 * 4 = 229376 bytes = 224 KB
    	bool bPointsUpdated = false;
    	bool bLinesUpdated = false;
    	uint lines_data_bindless_id;
    	uint points_data_bindless_id;
        BufferView<lines_data> lines_data_buffer;
        BufferView<point_data> points_data_buffer;
        unique_ptr<Shader2D<>> DrawLineShader;
        unique_ptr<Shader1D<>> DrawPointsShader;
    	vector<point_data> Points;
    	vector<lines_data> Lines;
    	map<uint, uint> PointIdToIndex;


    };
}