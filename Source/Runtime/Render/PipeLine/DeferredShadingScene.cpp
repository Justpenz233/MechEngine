//
// Created by Mayn on 2024/11/2.
//

#include "DeferredShadingScene.h"

#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/material/shading_function.h"
#include "Render/sampler/sampler_base.h"

namespace MechEngine::Rendering
{

void DeferredShadingScene::render_main_view(const UInt& frame_index, const UInt& time)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	get_sampler()->init(pixel_coord, frame_index);

	// Fill with default
	g_buffer.set_default(pixel_coord, make_float4(1.f));

	// Ray trace rasterization
	auto pixel_pos = make_float2(pixel_coord) + .5f;
	auto ray = view->generate_ray(pixel_pos);
	frame_buffer()->write(pixel_coord, make_float4(render_pixel(ray, pixel_pos), 1.f));
}

std::pair<Float3, Float> DeferredShadingScene::calc_surface_point_color(
	Var<Ray> ray, const ray_intersection& intersection, bool global_illumination)
{
	Float3 pixel_radiance = make_float3(0.f);
	Float Alpha = 1.f;
	$if(intersection.shape->is_light())
	{
		pixel_radiance = LightProxy->get_light_data(intersection.shape.light_id)->light_color;
	}
	$elif (intersection.shape->is_mesh()) // Surface shade
	{
		const auto& frame = intersection.shading_frame;
		auto material_data = MaterialProxy->get_material_data(intersection.material_id);
		Alpha = material_data.alpha;
		/************************************************************************
		 *								Shading
		 ************************************************************************/
		auto x = intersection.position_world;
		auto w_o = normalize(-ray->direction());

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
		// g_buffer.write(pixel_coord, bxdf_parameters, intersection);

		auto normal_world = bxdf_parameters.normal;

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
			return x + frame.local_to_world(triangle_interpolate(intersection.barycentric, tmpu, tmpv, tmpw));
		};
		auto shadow_ray_origin = bShadowRayOffset ? offset_ray() : offset_ray_origin(x, normal_world);


		$for(light_id, 128)
		{
			// First calculate light color, as rendering equation is L_i(x, w_i)
			auto light_data = LightProxy->get_light_data(light_id);
			$if(!light_data->valid()) {$break;};
			auto light_location = rtAccel->instance_transform(light_data->instance_id)[3].xyz();
			auto light_dir = normalize(light_location - x);

			auto calc_lighting = [&](const Float3& w_i, bool calc_shadow) {
				Float3 light_visibility = make_float3(1.);
				auto local_w_i = frame.world_to_local(w_i);
				auto local_w_o = frame.world_to_local(w_o);
				auto shading = def(make_float3(0.f));
				$if(dot(w_i, normal_world) >= 0.f)
				{
					if (calc_shadow)// Shadow ray
					{
						auto Hit = trace_closest(make_ray(shadow_ray_origin, w_i, 0.f, 1.f)).instance_id;
						light_visibility = select(make_float3(0.), make_float3(1.),
						 Hit == light_data->instance_id | Hit == ~0u);
					}

					// Dispatch light evaluate polymorphic, so that we can have different light type
					LightProxy->light_virtual_call.dispatch(
						light_data.light_type, [&](const light_base* light) {
							auto light_color  = light->l_i_rt(light_data, x, w_i, w_o, normal_world);
							MaterialProxy->shader_call.dispatch(
							material_data.shader_id, [&](const shader_base* material) {
								auto mesh_color = material->bxdf(bxdf_parameters, local_w_o, local_w_i);
								shading = mesh_color * light_color * max(dot(w_i, normal_world), 0.001f) * light_visibility;
							});
						});
				};
				return shading;
			};

			auto lighting = calc_lighting(light_dir, bRenderShadow);
			pixel_radiance += lighting;
		};
		if(global_illumination)
		{
			$if(material_data.alpha == 1.f) // Only reflect for opaque surface
			{
				Float3 reflect = make_float3(0.f);
				$for(spp, 0u, SamplePerPixel)
				{
					// Reflection by Orthonormal Basis
					// See https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#orthonormalbases
					auto reflect_dir = frame.local_to_world(sample_uniform_hemisphere_surface(get_sampler()->generate_2d()));
					auto reflect_ray = make_ray(offset_ray_origin(x, normal_world), reflect_dir);
					auto reflect_intersection = intersect(reflect_ray);
					$if(reflect_intersection.valid())
					{
						auto [reflect_radiance, alpha]
						= calc_surface_point_color(reflect_ray, reflect_intersection, false);
						reflect += reflect_radiance;
					};
				};
				pixel_radiance = pixel_radiance * 0.95f + reflect / Float(SamplePerPixel) * 0.05f;
			};
		}
	};
	return {pixel_radiance, Alpha};
}

Float3 DeferredShadingScene::render_pixel(Var<Ray> ray, const Float2& pixel_pos)
{
	auto intersection = intersect(ray);
	auto wireframe_intersection = intersection; // For wireframe pass
	Float  transmission = 1.f;
	Float3 pixel_color = make_float3(0.f);
	$while(intersection.valid())
	{
		auto [surface_radiance, alpha]
		= calc_surface_point_color(ray, intersection, bGlobalIllumination);

		pixel_color += surface_radiance * alpha * transmission;
		transmission *= 1.f - alpha;
		$if(alpha != 1.f & transmission > 1e-2f)
		{
			ray->set_origin(
				offset_ray_origin(intersection.position_world, intersection.triangle_normal_world, ray->direction()));
			intersection = intersect(ray);
			$continue;
		};
		$break;
	};
	pixel_color += transmission * BackgroundColor;

	// Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	// Currently, we only draw the first intersection with the wireframe, which means
	// the wireframe would disappear in refractive or alpha blended surface
	//@see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	//@see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	$if(wireframe_intersection.shape->is_mesh())
	{
		auto material_data = MaterialProxy->get_material_data(wireframe_intersection.material_id);
		$if(material_data->show_wireframe == 1)
		{
			auto view = CameraProxy->get_main_view();
			auto vertex_data = StaticMeshProxy->get_vertices(
				wireframe_intersection.shape->mesh_id,
				wireframe_intersection.primitive_id);
			auto transform = get_instance_transform(
				wireframe_intersection.instance_id);

			auto d = distance_to_triangle(pixel_pos,
				view->world_to_screen((transform * make_float4(vertex_data[0]->position(), 1.f)).xyz()),
				view->world_to_screen((transform * make_float4(vertex_data[1]->position(), 1.f)).xyz()),
				view->world_to_screen((transform * make_float4(vertex_data[2]->position(), 1.f)).xyz())
				);

			//@see https://backend.orbit.dtu.dk/ws/portalfiles/portal/3735323/wire-sccg.pdf
			auto wireframe_intensity = exp2(-2.f * square(d)); // I = exp2(-2 * d^2)
			pixel_color = lerp(pixel_color, make_float3(0.f), wireframe_intensity);
		};
	};
	return pixel_color;
}
};