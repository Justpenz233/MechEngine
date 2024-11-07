//
// Created by Mayn on 2024/11/2.
//

#include "PathTracingScene.h"

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
		color += render_path_tracing(ray, pixel_pos, pixel_coord);
	};
	color = color / Float(SamplePerPixel);
	g_buffer.linear_color->write(pixel_coord, make_float4(color, 1.f));
	frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(tone_mapping_aces(color)), 1.f));
}

Float3 PathTracingScene::render_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight)
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
			first_intersection = intersection;
		}; // For wireframe pass
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
			auto material_data = MaterialProxy->get_material_data(intersection.material_id);

			// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
			bxdf_context context{
				.intersection = intersection,
				.material_data = MaterialProxy->get_material_data(intersection.material_id),
			};

			material_parameters bxdf_parameters;
			MaterialProxy->shader_call.dispatch(
				material_data.shader_id, [&](const shader_base* material) {
					bxdf_parameters = material->calc_material_parameters(context);
				});

			// normal in world space
			auto normal = bxdf_parameters.normal;

			// Raytracing gem 2,  CHAPTER 4 HACKING THE SHADOW TERMINATOR
			auto offset_ray = [&]() {
				auto vps = StaticMeshProxy->get_vertices(intersection.shape->mesh_id, intersection.primitive_id);
				auto tmpu = x - vps[0]->position();
				auto tmpv = x - vps[1]->position();
				auto tmpw = x - vps[2]->position();

				auto dotu = min(0.f, dot(tmpu, vps[0]->normal()));
				auto dotv = min(0.f, dot(tmpv, vps[1]->normal()));
				auto dotw = min(0.f, dot(tmpw, vps[2]->normal()));

				tmpu -= dotu * vps[0]->normal();
				tmpv -= dotv * vps[1]->normal();
				tmpw -= dotw * vps[2]->normal();
				return x + triangle_interpolate(intersection.barycentric, tmpu, tmpv, tmpw);
			};
			auto shadow_ray_origin = bShadowRayOffset ? offset_ray() : offset_ray_origin(x, normal);

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
						MaterialProxy->shader_call.dispatch(
							material_data.shader_id, [&](const shader_base* material) {
								auto local_wi = frame.world_to_local(light_sample.w_i);
								auto brdf = material->bxdf(bxdf_parameters, local_wo, local_wi);
								auto pdf = material->pdf(bxdf_parameters, local_wo, local_wi);
								auto w = balance_heuristic(light_sample.pdf, pdf) / light_sample.pdf;
								pixel_radiance += w * beta * brdf * light_sample.l_i;
							});
					};
				});
			// Sample brdf
			MaterialProxy->shader_call.dispatch(
				material_data.shader_id, [&](const shader_base* material) {
					auto [local_wi, pdf] = material->sample(bxdf_parameters, local_wo, get_sampler()->generate_2d());
					auto world_wi = frame.local_to_world(local_wi);
					ray = make_ray(shadow_ray_origin, world_wi);
					pdf_bsdf = pdf;
					auto w = ite(pdf > 0.f, 1.f / pdf, 0.f);
					beta *= w * material->bxdf(bxdf_parameters, local_wo, local_wi);
				});
		};
	};


	$if(first_intersection.valid())
	{
		pixel_radiance = reproject_last_frame(first_intersection, pixel_coord, pixel_radiance);
		g_buffer.write(pixel_coord, first_intersection);
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

Float3 PathTracingScene::reproject_last_frame(const ray_intersection& intersection, const UInt2& pixel_coord, const Float3& pixel_color)
{
	auto transform_data = TransformProxy->get_transform_data(intersection.instance_id);
	auto& x = intersection.position_world;
	auto& pre_t = transform_data->last_transform_matrix;;
	auto& t_inv = transform_data->inverse_transform_matrix;

	auto pre_x = pre_t * t_inv * make_float4(x, 1.f);
	auto pre_clip_position = CameraProxy->get_main_view().last_view_projection_matrix * pre_x;
	auto pre_ndc_position = (pre_clip_position / pre_clip_position.w).xyz();
	UInt2 pre_pixel_coord = CameraProxy->get_main_view()->ndc_to_screen(pre_ndc_position);
	auto pre_color = g_buffer.linear_color->read(pre_pixel_coord);

	Float mixRate = 0.f;
	// Float3 result = pixel_color;
	$if (pre_ndc_position.x > -1.f & pre_ndc_position.x < 1.0f & pre_ndc_position.y > -1.f & pre_ndc_position.y < 1.0f)
	{
		mixRate = ite(g_buffer.instance_id->read(pre_pixel_coord).x == intersection.instance_id, 0.8f, 0.0f);
		// result = lerp(pixel_color, pre_color.xyz(), mixRate);
		// $if(all(pre_pixel_coord == pixel_coord))
		// {
			// result = make_float3(1.f, 0.f, 0.f);
		// };
	};

	// return result;
	return lerp(pixel_color, pre_color.xyz(), mixRate);
}

};