//
// Created by MarvelLi on 2024/4/29.
//

#include "LineSceneProxy.h"

#include "CameraSceneProxy.h"
#include "Render/Core/transform_data.h"
#include "Render/Core/view_data.h"
#include "Render/Core/math_function.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
    LineSceneProxy::LineSceneProxy(RayTracingScene& InScene) noexcept
	: SceneProxy(InScene)
	{
		lines_data_buffer = Scene.RegisterBuffer<lines_data>(MaxCurveCount);
		points_data_buffer = Scene.RegisterBuffer<point_data>(MaxPointCount);
    	Points.reserve(MaxPointCount);
	}


	uint LineSceneProxy::AddPoint(float3 WorldPosition, float Radius, float3 color)
	{
    	bPointsUpdated = true;
    	Points.push_back({WorldPosition, Radius, color});
    	return Points.size() - 1;
	}

	void LineSceneProxy::UploadDirtyData(Stream& stream)
    {
    	if (bPointsUpdated)
		{
			stream << points_data_buffer.subview(0, Points.size()).copy_from(Points.data());
			bPointsUpdated = false;
		}
    }

	void LineSceneProxy::CompileShader()
	{
		DrawPointsShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
			[&](Var<view_data> view) {
				auto point_id = dispatch_id().x;
				auto point = points_data_buffer->read(point_id);
				auto world_position = point.world_position;
				auto screen_position = view->world_to_screen(world_position);

				auto pixel_min_int =
					make_uint2(UInt(screen_position.x - 0.5f - point.radius),
						UInt(screen_position.y - 0.5f - point.radius));
				pixel_min_int = max(pixel_min_int, make_uint2(0, 0));

				auto pixel_max_int =
					make_uint2(UInt(screen_position.x + 0.5f + point.radius),
					UInt(screen_position.y + 0.5f + point.radius));
				pixel_max_int = min(pixel_max_int, make_uint2(view->viewport_size.x, view->viewport_size.y));

				auto radius_square = square(point.radius);
				$for(x, pixel_min_int.x, pixel_max_int.x)
				{
					$for(y, pixel_min_int.y, pixel_max_int.y)
					{
						auto pixel = make_uint2(x, y);
						auto distance = square(x - screen_position.x) + square(y - screen_position.y);
						$if(distance < radius_square)
						{
							Scene.frame_buffer()->write(pixel, make_float4(point.color, 1.f));
						};
					};
				};
			}));
	}

	void LineSceneProxy::PostRenderPass(Stream& stream)
	{
		SceneProxy::PostRenderPass(stream);
    	if (!Points.empty())
    	{
    		stream << (*DrawPointsShader)(Scene.GetCameraProxy()->GetCurrentViewData()).dispatch(Points.size());
    	}
	}
}
