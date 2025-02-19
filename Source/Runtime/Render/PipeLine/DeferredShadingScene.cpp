//
// Created by Mayn on 2024/11/2.
//

#include "DeferredShadingScene.h"
#include "Mesh/StaticMesh.h"
#include "Misc/Config.h"
#include "Render/Core/frame.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/sampler/sampler_base.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/scanline_rasterizer.h"

namespace MechEngine::Rendering
{

DeferredShadingScene::DeferredShadingScene(
	Stream& stream, Device& device, ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept
	: GpuScene(stream, device, InWindows, InViewport) {}

DeferredShadingScene::~DeferredShadingScene()
{
}

void DeferredShadingScene::LoadRenderSettings()
{
	GpuScene::LoadRenderSettings();
	bGlobalIllumination = GConfig.Get<bool>("DeferredShading", "GlobalIllumination");
	bRenderShadow = GConfig.Get<bool>("DeferredShading", "RenderShadow");
}

void DeferredShadingScene::PrePass(CommandList& CmdList)
{
	GpuScene::PrePass(CmdList);

	if(bUseRasterizer)
	{
		auto MeshSceneProxy = StaticMeshProxy.get();

		Rasterizer->ClearPass(CmdList);

		// Iterate all the mesh in the scene
		for(auto [MeshId, Mesh] : MeshSceneProxy->MeshIdToPtr)
		{
			ASSERT(Mesh != nullptr);
			for(auto instance_id : MeshSceneProxy->MeshInstances[MeshId])
			{
				Rasterizer->VisibilityPass(CmdList, instance_id, MeshId,
					Mesh->GetVertexNum(), Mesh->GetFaceNum(), Mesh->IsBackFaceCulling());
			}
		}
		stream << CmdList.commit();
	}
}

void DeferredShadingScene::render_main_view(const UInt& frame_index, const UInt& time)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	g_buffer.set_default(pixel_coord);
	if (bGlobalIllumination)
		get_sampler()->init(pixel_coord, frame_index);

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
		auto light_data = LightProxy->get_light_data(intersection.shape.light_id);
		pixel_radiance = light_data->light_color * light_data->intensity;
	}
	$elif (intersection.shape->is_mesh()) // Surface shade
	{
		auto frame = frame::make(intersection.corner_normal_world);
		auto material_data = MaterialProxy->get_material_data(intersection.material_id);
		Alpha = material_data.alpha;
		/************************************************************************
		 *								Shading
		 ************************************************************************/
		auto x = intersection.position_world;
		auto w_o = normalize(-ray->direction());

		$comment("Read material parameters");
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
		static Callable offset_ray = [&]() {
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

		$comment("For each light source");
		$for(light_id, 128)
		{
			// First calculate light color, as rendering equation is L_i(x, w_i)
			auto light_data = LightProxy->get_light_data(light_id);
			$if(!light_data->valid()) {$break;};
			auto light_location = rtAccel->instance_transform(light_data->instance_id)[3].xyz();
			auto light_dir = normalize(light_location - x);

			Callable calc_lighting = [&](const Float3& w_i) {
				$comment("Calculate lighting");
				Float3 light_visibility = make_float3(1.);
				auto local_w_i = frame.world_to_local(w_i);
				auto local_w_o = frame.world_to_local(w_o);
				auto shading = def(make_float3(0.f));
				$if(dot(w_i, normal_world) >= 0.f)
				{
					if (bRenderShadow)// Shadow ray
					{
						$comment("render shadow");
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
								$comment("Calculate bxdf shading");
								auto mesh_color = material->bxdf(bxdf_parameters, local_w_o, local_w_i);
								shading = mesh_color * light_color * max(dot(w_i, normal_world), 0.001f) * light_visibility;
							});
						});
				};
				return shading;
			};

			$comment("Calculate lighting");
			auto lighting = calc_lighting(light_dir);
			pixel_radiance += lighting;
		};
		if(global_illumination)
		{
			$comment("global illumination");
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

Float3 DeferredShadingScene::render_pixel(Var<Ray> ray, const UInt2& pixel_coord)
{
	ray_intersection intersection;
	$comment("Rasterization");

	if(bUseRasterizer)
	{
		auto instance_id = Rasterizer->vbuffer.instance_id->read(pixel_coord).x;
		auto triangle_id = Rasterizer->vbuffer.triangle_id->read(pixel_coord).x;
		auto bary = Rasterizer->vbuffer.bary->read(pixel_coord).xy();
		intersection = intersect({instance_id, triangle_id, bary}, ray);
	}
	else
	{
		intersection = intersect(ray);
	}

	$if(intersection.valid()) { g_buffer.write(pixel_coord, intersection)	;};

	Float  transmission = 1.f;
	Float3 pixel_color = make_float3(0.f);
	$comment("Trace path if transparent");
	$while(intersection.valid())
	{
		auto [surface_radiance, alpha]
		= calc_surface_point_color(ray, intersection, bGlobalIllumination);

		$comment("accumulate color");
		pixel_color += surface_radiance * alpha * transmission;
		transmission *= 1.f - alpha;
		$if(alpha != 1.f & transmission > 1e-2f)
		{
			$comment("Transparent surface, trace next ray");
			ray->set_origin(
				offset_ray_origin(intersection.position_world, intersection.triangle_normal_world, ray->direction()));
			intersection = intersect(ray);
			$continue;
		};
		$break;
	};
	pixel_color += transmission * BackgroundColor;
	return pixel_color;
}
};
