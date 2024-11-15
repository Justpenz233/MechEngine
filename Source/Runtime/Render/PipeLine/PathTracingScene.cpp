//
// Created by Mayn on 2024/11/2.
//

#include "PathTracingScene.h"

#include "Render/Core/shadow_terminator.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/material/shading_function.h"
#include "Render/sampler/sampler_base.h"
#include "Render/Core/transform_data.h"

namespace MechEngine::Rendering
{
void PathTracingScene::InitBuffers()
{
	GpuScene::InitBuffers();
	auto WindowSize = Window->framebuffer().size();
	pre_linear_color = device.create_image<float>(PixelStorage::FLOAT4,
		WindowSize.x, WindowSize.y);
	pre_world_position = device.create_image<float>(PixelStorage::FLOAT4,
		WindowSize.x, WindowSize.y);

	reservoirs = device.create_buffer<ris_reservoir>(WindowSize.x * WindowSize.y);
}

void PathTracingScene::render_main_view(const UInt& frame_index, const UInt& time)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	get_sampler()->init(pixel_coord, time);

	auto pixel_pos = make_float2(pixel_coord) + 0.5f;

	auto ray = view->generate_ray(pixel_pos); Float3 color = make_float3(0.f);
	$for(Sample, 0u, def(SamplePerPixel))
	{
		color += mis_path_tracing(ray, pixel_pos, pixel_coord);
	};
	color = color / Float(SamplePerPixel);
	pre_linear_color->write(pixel_coord, make_float4(color, 1.f));
	frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(tone_mapping_aces(color)), 1.f));
}

Float3 PathTracingScene::mis_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight)
{
	ray_intersection first_intersection;
	Float3			 pixel_radiance = make_float3(0.f);
	Float3			 beta = make_float3(1.f);
	auto			 pdf_bsdf = def(1e16f);

	$for(depth, 0, 2)
	{
		auto		intersection = intersect(ray);
		const auto& frame = intersection.shading_frame;
		const auto& x = intersection.position_world;
		const auto& w_o = normalize(-ray->direction());
		auto		local_wo = frame.world_to_local(w_o);
		$if(depth == 0)
		{
			first_intersection = intersection; // For wireframe pass
		};
		$if(!intersection.valid())
		{
			$break;
		};
		$if(intersection.shape->has_light() & depth == 0)
		{
			auto light = LightProxy->get_light_data(intersection.shape.light_id);
			pixel_radiance = light->light_color;
			$break;
		};
		$if(intersection.shape->has_light() & depth != 0)
		{
			auto light = LightProxy->get_light_data(intersection.shape.light_id);
			LightProxy->light_virtual_call.dispatch(light.light_type,
				[&](const light_base* light_type) {
					auto [f, pdf] = light_type->l_i(light, ray->origin(), x);
					pixel_radiance += beta * f * balance_heuristic(pdf_bsdf, pdf);
				});
			$break;
		};
		$if(intersection.shape->has_surface()) // Surface shade
		{
			// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
			auto [shader_id, bxdf_parameters] = MaterialProxy->get_material_parameters(intersection);

			// normal in world space
			auto normal = bxdf_parameters.normal;
			auto shadow_ray_origin = offset_ray_origin(x, normal);

			// Sample light
			auto light_id = 0;
			auto light = LightProxy->get_light_data(light_id);
			LightProxy->light_virtual_call.dispatch(light.light_type,
				[&](const light_base* light_type) {
					auto light_sample = light_type->sample_li(light, x, get_sampler()->generate_2d());
					auto occluded = has_hit(make_ray(x, light_sample.w_i, 0.01f, 0.99f));
					auto cos = dot(normalize(light_sample.w_i), normal);
					$if(cos > 0.01f & !occluded & light_sample.pdf > 0.f)
					{
						auto local_wi = frame.world_to_local(light_sample.w_i);
						auto [brdf, pdf] = MaterialProxy->brdf_pdf(shader_id, bxdf_parameters, local_wo, local_wi);
						auto w = balance_heuristic(light_sample.pdf, pdf) / light_sample.pdf;
						pixel_radiance += w * beta * brdf * light_sample.l_i;
					};
				});

			// Sample brdf
			{
				auto [local_wi, brdf, pdf]
				= MaterialProxy->sample_brdf(shader_id, bxdf_parameters, local_wo, get_sampler()->generate_2d());
				auto world_wi = frame.local_to_world(local_wi);
				ray = make_ray(shadow_ray_origin, world_wi);
				pdf_bsdf = pdf;
				auto w = ite(pdf > 0.f, 1.f / pdf, 0.f);
				beta *= w * brdf;
			}
		};
	};
	$if(first_intersection.valid())
	{
		pixel_radiance = reproject_last_frame(first_intersection, pixel_coord, pixel_radiance);
		g_buffer.write(pixel_coord, first_intersection);
		pre_world_position->write(pixel_coord, make_float4(first_intersection.position_world, 1.f));
		auto wireframe = wireframe_intensity(first_intersection, pixel_pos);
		pixel_radiance = lerp(pixel_radiance, wireframe_color, wireframe);
	};
	return pixel_radiance;
}


Float PathTracingScene::wireframe_intensity(const ray_intersection& intersection, const Float2& pixel_pos) const
{
	// Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	// Currently, we only draw the first intersection with the wireframe, which means
	// the wireframe would disappear in refractive or alpha blended surface
	//@see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	//@see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	auto wireframe_intensity = def(0.f);
	$if(intersection.shape->is_mesh())
	{
		auto material_data = MaterialProxy->get_material_data(intersection.material_id);
		$if(material_data->show_wireframe == 1)
		{
			auto view = CameraProxy->get_main_view();
			auto vertex_data = StaticMeshProxy->get_vertices(
				intersection.shape->mesh_id,
				intersection.primitive_id);
			auto transform = get_instance_transform(
				intersection.instance_id);

			auto d = distance_to_triangle(pixel_pos,
				view->world_to_screen((transform * make_float4(vertex_data[0]->position(), 1.f)).xyz()),
				view->world_to_screen((transform * make_float4(vertex_data[1]->position(), 1.f)).xyz()),
				view->world_to_screen((transform * make_float4(vertex_data[2]->position(), 1.f)).xyz()));

			//@see https://backend.orbit.dtu.dk/ws/portalfiles/portal/3735323/wire-sccg.pdf
			wireframe_intensity = exp2(-2.f * square(d)); // I = exp2(-2 * d^2)
		};
	};
	return wireframe_intensity;
}

// @see https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl#L93
// re-project then bi-linear interpolate the last frame color
Float3 PathTracingScene::reproject_last_frame(const ray_intersection& intersection, const UInt2& pixel_coord, const Float3& pixel_color)
{
	const float NORMAL_TOLERANCE = 5.0e-2;
	const float POSITION_TOLERANCE = 1e-2;

	auto transform_data = TransformProxy->get_instance_transform_data(intersection.instance_id);
	auto& x = intersection.position_world;
	auto& pre_t = transform_data->last_transform_matrix;;
	auto& t_inv = transform_data->inverse_transform_matrix;

	auto pre_x = pre_t * t_inv * make_float4(x, 1.f);
	auto pre_clip_position = CameraProxy->get_main_view().last_view_projection_matrix * pre_x;
	auto pre_ndc_position = (pre_clip_position / pre_clip_position.w).xyz();

	Float2 pre_pixel_pos = CameraProxy->get_main_view()->ndc_to_screen(pre_ndc_position) - 0.5f;
	UInt2 pre_pixel_coord = UInt2(pre_pixel_pos);

	auto pixel_pos_frac = pre_pixel_pos - make_float2(pre_pixel_coord);
	auto inv_pixel_pos_frac = 1.f - pixel_pos_frac;
	Float weights[4];
	weights[0] = inv_pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[1] = pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[2] = inv_pixel_pos_frac.x * pixel_pos_frac.y;
	weights[3] = pixel_pos_frac.x * pixel_pos_frac.y;

	Float3 sum_val = make_float3(0.f);
	Float sum_weight = 0.f;
	auto WinSize = GetWindosSize();
	for(int dy = 0; dy <= 1; dy += 1)
	{
		for (int dx = 0; dx <= 1; dx += 1)
		{
			auto p = make_uint2(dx, dy) + pre_pixel_coord;
			$if (all(p > make_uint2(0, 0)) & all(p < WinSize))
			{
				auto pre_instance = g_buffer.instance_id->read(p).x;
				auto pre_world_pos = pre_world_position->read(p).xyz();
				auto pre_normal = g_buffer.normal->read(p).xyz();
				$if(pre_instance == intersection.instance_id &
					distance_squared(pre_world_pos, x) < POSITION_TOLERANCE &
					distance_squared(pre_normal, intersection.corner_normal_world) < NORMAL_TOLERANCE)
				{
					auto pre_color = pre_linear_color->read(p);
					sum_weight += weights[dx + dy * 2];
					sum_val += pre_color.xyz() * weights[dx + dy * 2];
				};
			};
		}
	}
	sum_val /= ite(sum_weight > 0.f, sum_weight, 1.f);
	return ite(sum_weight > 0.f, lerp(pixel_color, sum_val, 0.8f), pixel_color);
}

};