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
		lines_data_buffer = Scene.RegisterBuffer<lines_data>(MaxLinesCount);
		points_data_buffer = Scene.RegisterBuffer<point_data>(MaxPointCount);
    	Points.reserve(MaxPointCount);
    	Lines.reserve(MaxLinesCount);
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
			stream << points_data_buffer.copy_from(Points.data());
			bPointsUpdated = false;
		}
    	if (bLinesUpdated)
		{
			stream << lines_data_buffer.copy_from(Lines.data());
			bLinesUpdated = false;
		}
	}

	uint LineSceneProxy::AddLine(float3 WorldStart, float3 WorldEnd, float Thickness, float3 Color)
	{
    	bLinesUpdated = true;
    	Lines.push_back({WorldStart, WorldEnd, Thickness, Color});
    	return Lines.size() - 1;
	}


	void LineSceneProxy::CompileShader()
	{

    	Callable raster_point = [&](Var<view_data> view, Float3 screen_position, Float radius, Float3 color) {
    		auto pixel_min_int =
					make_uint2(UInt(screen_position.x - 0.5f - radius),
						UInt(screen_position.y - 0.5f - radius));
    		pixel_min_int = max(pixel_min_int, make_uint2(0, 0));

    		auto pixel_max_int =
				make_uint2(UInt(screen_position.x + 0.5f + radius),
				UInt(screen_position.y + 0.5f + radius));
    		pixel_max_int = min(pixel_max_int, make_uint2(view->viewport_size.x, view->viewport_size.y));

    		auto radius_square = square(radius);
    		$for(x, pixel_min_int.x, pixel_max_int.x)
    		{
    			$for(y, pixel_min_int.y, pixel_max_int.y)
    			{
    				auto pixel = make_uint2(x, y);
    				auto distance = square(x - screen_position.x) + square(y - screen_position.y);
    				$if(distance < radius_square)
    				{
    					Scene.frame_buffer()->write(pixel, make_float4(color, 1.f));
    				};
    			};
    		};
    	};
		DrawPointsShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
			[&](Var<view_data> view) {
				auto point_id = dispatch_id().x;
				auto point = points_data_buffer->read(point_id);
				auto world_position = point.world_position;
				auto ndc_position = view->world_to_ndc(world_position);
				auto screen_position = view->ndc_to_screen(ndc_position);
				raster_point(view, make_float3(screen_position, ndc_position.z), point.radius, point.color);
			}));


    	DrawLineShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
			[&](Var<view_data> view) {
				auto line_id = dispatch_id().x;
				auto line = lines_data_buffer->read(line_id);
				auto ndc_start = view->world_to_ndc(line.world_start);
				auto ndc_end = view->world_to_ndc(line.world_end);
				auto screen_start = view->ndc_to_screen(ndc_start);
				auto screen_end = view->ndc_to_screen(ndc_end);
				auto thickness = line.thickness;

				Bool Steep = false;
				$if(abs(screen_start.x - screen_end.x) < abs(screen_start.y - screen_end.y))
				{
					swap(screen_start.x, screen_start.y);
					swap(screen_end.x, screen_end.y);
					Steep = true;
				};
				$if(screen_start.x > screen_end.x)
				{
					swap(ndc_start, ndc_end);
					swap(screen_start, screen_end);
				};
				Int dx = screen_end.x - screen_start.x;
				Int dy = screen_end.y - screen_start.y;
				Int derror2 = abs(dy) * 2;
				Int error = 0;

				Int y = screen_start.y;
				Float t = 0.f;

				$for(x, Int(screen_start.x), Int(screen_end.x))
				{
					t = Float(x - screen_start.x) / Float(dx);
					auto pixel_pos = make_float3((Float)x, (Float)y, lerp(ndc_start.z, ndc_end.z, t));
					$if (Steep)
					{
						swap(pixel_pos.x, pixel_pos.y);
					};
					raster_point(view, pixel_pos, thickness, line.color);

					error += derror2;
					$if(error > dx)
					{
						y += select( -1 , 1, dy > 0);
						error -= dx * 2;
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
    	if (!Lines.empty())
		{
			stream << (*DrawLineShader)(Scene.GetCameraProxy()->GetCurrentViewData()).dispatch(Lines.size());
		}
	}
}
