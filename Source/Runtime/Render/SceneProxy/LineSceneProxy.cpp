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
		std::tie(lines_data_buffer, lines_data_bindless_id) = Scene.RegisterBindlessBuffer<lines_data>(MaxLinesCount);
		std::tie(points_data_buffer, points_data_bindless_id) = Scene.RegisterBindlessBuffer<point_data>(MaxPointCount);
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
			if (Points.size() > points_data_buffer.size())
			{
				points_data_buffer = Scene.RegisterBuffer<point_data>(Points.capacity());
				Scene.GetBindlessArray().emplace_on_update(points_data_bindless_id, points_data_buffer);
				ASSERT(points_data_buffer.size() == Points.capacity());
				LOG_WARNING("Point render buffer is full, resize to {0}", Points.capacity());
				LOG_WARNING("Current point  data buffer size: {0} MB", points_data_buffer.size_bytes() / 1024. / 1024.);
			}

			stream << points_data_buffer.subview(0, Points.size()).copy_from(Points.data());
			bPointsUpdated = false;
		}
    	if (bLinesUpdated)
		{
    		if (Lines.size() > lines_data_buffer.size())
    		{
    			lines_data_buffer = Scene.RegisterBuffer<lines_data>(Lines.capacity());
    			Scene.GetBindlessArray().emplace_on_update(lines_data_bindless_id, lines_data_buffer);
    			ASSERT(lines_data_buffer.size() == Lines.capacity());
    			LOG_WARNING("Line render buffer is full, resize to {0}", Lines.capacity());
    			LOG_WARNING("Current line data buffer size: {0} MB", lines_data_buffer.size_bytes() / 1024. / 1024.);
    		}

			stream << lines_data_buffer.subview(0, Lines.size()).copy_from(Lines.data()) << synchronize();
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
    		$for(x, pixel_min_int.x, pixel_max_int.x + 1)
    		{
    			$for(y, pixel_min_int.y, pixel_max_int.y + 1)
    			{
    				auto pixel = make_uint2(x, y);
    				auto distance = square(x - screen_position.x) + square(y - screen_position.y);
    				$if(distance < radius_square)
    				{
    					auto depth_index = Scene.get_gbuffer().flattend_index(pixel);
    					auto& depth_buffer = Scene.get_gbuffer().depth;
    					depth_buffer->atomic(depth_index).fetch_min(screen_position.z);
    					$if(depth_buffer->read(depth_index) == screen_position.z)
						{
							Scene.frame_buffer()->write(pixel, make_float4(color, 1.f));
						};
    				};
    			};
    		};
    	};
		DrawPointsShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
			[&](Var<view_data> view) {
				auto point_id = dispatch_id().x;
				auto point = bindelss_buffer<point_data>(points_data_bindless_id)->read(point_id);
				auto world_position = point.world_position;
				auto ndc_position = view->world_to_ndc(world_position);
				auto screen_position = view->ndc_to_screen(ndc_position);
				raster_point(view, make_float3(screen_position, ndc_position.z), point.radius, point.color);
			}));


    	DrawLineShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
			[&](Var<view_data> view) {
				auto line_id = dispatch_id().x;
				auto line = bindelss_buffer<lines_data>(lines_data_bindless_id)->read(line_id);
				auto ndc_start = view->world_to_ndc(line.world_start);
				auto ndc_end = view->world_to_ndc(line.world_end);
				$if(!(ndc_start.z < 0.f & ndc_end.z < 0.f) &
					!(ndc_start.x > 0.5f & ndc_end.x > 0.5f) &
					!(ndc_start.x < -0.5f & ndc_end.x < -0.5f) &
					!(ndc_start.y > 0.5f & ndc_end.y > 0.5f) &
					!(ndc_start.y < -0.5f & ndc_end.y < -0.5f))
				{
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

					Float t = 0.f;
					Float2 clip_screen_start = screen_start, clip_screen_end = screen_end;
					// clip line into screen box
					$if(Steep)
					{
						swap(clip_screen_start.x, clip_screen_start.y);
						swap(clip_screen_end.x, clip_screen_end.y);
						std::tie(clip_screen_start, clip_screen_end)
						= view->clamp_to_screen(clip_screen_start, clip_screen_end);
						swap(clip_screen_start.x, clip_screen_start.y);
						swap(clip_screen_end.x, clip_screen_end.y);
					}
					$else
					{
						std::tie(clip_screen_start, clip_screen_end)
						= view->clamp_to_screen(screen_start, screen_end);
					};
					Int y = clip_screen_start.y;
					$for(x, Int(clip_screen_start.x), Int(clip_screen_end.x))
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
