//
// Created by MarvelLi on 2024/4/29.
//

#include "LineSceneProxy.h"

#include "CameraSceneProxy.h"
#include "Render/Core/transform_data.h"
#include "Render/Core/view.h"
#include "Render/Core/math_function.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{
    LineSceneProxy::LineSceneProxy(GpuScene& InScene) noexcept
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

    	// Draw a cirlce in 2d screen space simulating thickness
    	Callable raster_point = [&](Var<view> view, Float3 screen_position, Float radius, Float3 color) {
    		$comment("Draw a circle in 2d screen space simulating thickness");
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

    	// Bresenham's line algorithm
    	Callable raster_line = [&](Var<view> view, Float3 ndc_start, Float3 ndc_end, Float thickness, Float3 color) {
    		$comment("Bresenham's line algorithm");
    		auto screen_start = view->ndc_to_screen(ndc_start);
    		auto screen_end = view->ndc_to_screen(ndc_end);
    		Bool Steep = false;
    		$if(abs(screen_start.x - screen_end.x) < abs(screen_start.y - screen_end.y))
    		{
    			swap(screen_start.x, screen_start.y);
    			swap(screen_end.x, screen_end.y);
    			Steep = true;
    		};
    		$if(screen_start.x > screen_end.x)
    		{
    			compute::swap(ndc_start, ndc_end);
    			compute::swap(screen_start, screen_end);
    		};
    		Int dx = screen_end.x - screen_start.x;
    		Int dy = screen_end.y - screen_start.y;
    		Int derror2 = abs(dy) * 2;
    		Int error = 0;

    		Float t = 0.f;
    		Int y = screen_start.y;
    		$for(x, Int(screen_start.x), Int(screen_end.x))
    		{
    			t = Float(x - screen_start.x) / Float(dx);
    			auto pixel_pos = make_float3((Float)x, (Float)y, lerp(ndc_start.z, ndc_end.z, t));
    			$if (Steep)
    			{
    				swap(pixel_pos.x, pixel_pos.y);
    			};
    			raster_point(view, pixel_pos, thickness, color);

    			error += derror2;
    			$if(error > dx)
    			{
    				y += select( -1 , 1, dy > 0);
    				error -= dx * 2;
    			};
    		};
    	};

		DrawPointsShader = luisa::make_unique<Shader1D<>>(Scene.RegisterShader<1>(
			[&]() {
				$comment("DrawPointsShader");
				auto view = Scene.GetCameraProxy()->get_main_view();
				auto point_id = dispatch_id().x;
				auto point = bindelss_buffer<point_data>(points_data_bindless_id)->read(point_id);
				auto world_position = point.world_position;
				auto ndc_position = view->world_to_ndc(world_position);
				auto screen_position = view->ndc_to_pixel(ndc_position);
				raster_point(view, make_float3(screen_position, ndc_position.z), point.radius, point.color);
			}, "DrawPointShader"));

    	DrawLineShader = luisa::make_unique<Shader2D<>>(Scene.RegisterShader<2>(
			[&]() {
				$comment("DrawLineShader");
				auto view = Scene.GetCameraProxy()->get_main_view();
				auto line_id = dispatch_id().x;
				auto segment_id = dispatch_id().y;
				Float segments = Float(dispatch_size_y());
				auto line = bindelss_buffer<lines_data>(lines_data_bindless_id)->read(line_id);
				auto ndc_start = view->world_to_ndc(line.world_start);
				auto ndc_end = view->world_to_ndc(line.world_end);
				$if(!(ndc_start.z < -1.f & ndc_end.z < -1.f) &
					!(ndc_start.z > 1.f & ndc_end.z > 1.f) &
					!(ndc_start.x > 1.f & ndc_end.x > 1.f) &
					!(ndc_start.x < -1.f & ndc_end.x < -1.f) &
					!(ndc_start.y > 1.f & ndc_end.y > 1.f) &
					!(ndc_start.y < -1.f & ndc_end.y < -1.f))
				{
					std::tie(ndc_start, ndc_end) = view->clamp_to_ndc(ndc_start, ndc_end);
					auto segment_start = lerp(ndc_start, ndc_end, Float(segment_id) / segments);
					auto segment_end = lerp(ndc_start, ndc_end, Float(segment_id + 1) / segments);
					raster_line(view, segment_start, segment_end, line.thickness, line.color);
				};
			}, "DrawLineShader"));
	}

	void LineSceneProxy::PostRenderPass(CommandList& CmdList)
	{
    	static int NThreadPerLine = 16; // for each line, how many thread should be used
    	if (!Points.empty())
    	{
    		CmdList << (*DrawPointsShader)().dispatch(Points.size());
    	}
    	if (!Lines.empty())
		{
			CmdList << (*DrawLineShader)().dispatch(Lines.size(), NThreadPerLine);
		}
	}
}
