//
// Created by Mayn on 2024/11/2.
//

#include "PathTracingScene.h"

#include "Mesh/StaticMesh.h"
#include "Misc/Config.h"
#include "Render/Core/frame.h"
#include "Render/Core/shadow_terminator.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/sampler/sampler_base.h"
#include "Render/Core/transform_data.h"
#include "denoiser/denoiser_ext.h"
#include "denoiser/svgf.h"
#include "denoiser/denoiser.h"
#include "luisa/rust/api_types.hpp"
#include "rasterizer/rasterizer.h"

namespace MechEngine::Rendering
{

void PathTracingScene::LoadRenderSettings()
{
	GpuScene::LoadRenderSettings();
	bUseDenoiser = GConfig.Get<bool>("PathTracing", "Denoiser");
}
void PathTracingScene::InitPass(CommandList& CmdList)
{
	GpuScene::InitPass(CmdList);
	if (bUseDenoiser)
	{
		denoiser_ext = std::make_unique<denoiser>(this);
		denoiser_ext->InitPass(device, CmdList);
	}
}

void PathTracingScene::CompileShader()
{
	GpuScene::CompileShader();

	if (denoiser_ext)
		denoiser_ext->CompileShader(device, bShaderDebugInfo);
}

void PathTracingScene::PrePass(CommandList& CmdList)
{
	GpuScene::PrePass(CmdList);
	if (bUseRasterizer)
	{
		auto MeshSceneProxy = StaticMeshProxy.get();

		Rasterizer->ClearPass(CmdList);

		// Iterate all the mesh in the scene
		for (auto [MeshId, Mesh] : MeshSceneProxy->MeshIdToPtr)
		{
			ASSERT(Mesh != nullptr);
			for (auto instance_id : MeshSceneProxy->MeshInstances[MeshId])
			{
				Rasterizer->VisibilityPass(CmdList, instance_id, MeshId,
					Mesh->GetVertexNum(), Mesh->GetFaceNum(), Mesh->IsBackFaceCulling());
			}
		}
	}
}
void PathTracingScene::Render()
{
	CommandList CmdList{};
	PrePass(CmdList);
	CmdList << (*MainShader)(FrameCounter++, TimeCounter++).dispatch(GetWindosSize());

	if(denoiser_ext)
	{
		denoiser_ext->PostPass(CmdList);
		stream << CmdList.commit();
	}

	PostPass(CmdList);
	stream << CmdList.commit();
}

void PathTracingScene::render_main_view(const UInt& frame_index, const UInt& time)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	get_sampler()->init(pixel_coord, time);

	auto pixel_pos = make_float2(pixel_coord) + 0.5f;

	auto   ray = view->generate_ray(pixel_pos);
	Float3 color = make_float3(0.f);
	$for(Sample, 0u, def(SamplePerPixel)) // Maybe we assume 1spp for all case? because we are only implementing real-time rendering
	{
		color += mis_path_tracing(ray, pixel_pos, pixel_coord);
	};
	color = color / Float(SamplePerPixel);
	frame_buffer()->write(pixel_coord, make_float4(color, 1.f));
}

ray_intersection PathTracingScene::intersect_bias(const UInt2& pixel_coord, Expr<Ray> ray, Bool first_intersect)
{
	ray_intersection intersection;
	if(bUseRasterizer)
	{
		$if(first_intersect)
		{
			auto instance_id = Rasterizer->vbuffer.instance_id->read(pixel_coord).x;
			auto triangle_id = Rasterizer->vbuffer.triangle_id->read(pixel_coord).x;
			auto bary = Rasterizer->vbuffer.bary->read(pixel_coord).xy();
			intersection = intersect({instance_id, triangle_id, bary}, ray);

			// Restore the world position from depth buffer
			auto depth = g_buffer.read_depth(pixel_coord);
			auto view = CameraProxy->get_main_view();
			auto world_p = view->screen_to_world(make_float3(make_float2(pixel_coord) + 0.5f, depth));
			intersection.position_world = world_p;
			intersection.depth = depth;
		}
		$else
		{
			intersection = intersect(ray);
		};
	}
	else
	{
		intersection = intersect(ray);
	}
	return intersection;
}

Float3 PathTracingScene::mis_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight)
{
	ray_intersection first_intersection;
	Float3			 pixel_radiance = make_float3(0.f);
	Float3			 beta = make_float3(1.f);
	auto			 pdf_bsdf = def(1e16f);

	$for(depth, 0, 2)
	{
		auto		intersection = intersect_bias(pixel_coord, ray, depth == 0);
		const auto& x = intersection.position_world;

		$comment("If the intersection is invalid");
		$if(!intersection.valid())
		{
			$break;
		};

		$if (depth == 0)
		{
			first_intersection = intersection;
		};

		$comment("If the intersection is light");
		$if(intersection.shape->has_light())
		{
			$if(depth == 0)
			{
				auto light = LightProxy->get_light_data(intersection.shape.light_id);
				pixel_radiance = light->light_color * light->intensity;
			}
			$else{
				auto [li, pdf] = LightProxy->l_i(intersection.shape.light_id, ray->origin(), x);
				pixel_radiance += beta * li * balance_heuristic(pdf_bsdf, pdf);
			};
			$break;
		};

		$comment("Shading surface");
		$if(intersection.shape->has_surface()) // Surface shade
		{
			// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
			auto [shader_id, bxdf_parameters] = MaterialProxy->get_material_parameters(intersection);

			// normal in world space
			auto normal = bxdf_parameters.normal;
			auto shadow_ray_origin = offset_ray_origin(x, normal);
			const auto& w_o = normalize(-ray->direction());
			auto frame = frame::make(intersection.corner_normal_world);
			auto		local_wo = frame.world_to_local(w_o);

			$comment("Sample light");
			{
				auto light_id = 0;
				auto light_sample = LightProxy->sample_li(light_id, x, get_sampler()->generate_2d());
				light_sample.w_i = normalize(light_sample.w_i);
				auto occluded = has_hit(make_ray(x, light_sample.w_i, 0.01f, distance(light_sample.p_l, x) * 0.99f));
				auto cos = dot(normalize(light_sample.w_i), normal);
				$if(cos > 0.01f & !occluded & light_sample.pdf > 0.f)
				{
					auto local_wi = frame.world_to_local(light_sample.w_i);
					auto [brdf, pdf] = MaterialProxy->brdf_pdf(shader_id, bxdf_parameters, local_wo, local_wi);
					auto w = balance_heuristic(light_sample.pdf, pdf) / light_sample.pdf;
					pixel_radiance += w * beta * brdf * light_sample.l_i;
				};
			}

			// Sample brdf
			{
				$comment("Next event estimation");
				auto [local_wi, brdf, pdf]
				= MaterialProxy->sample_brdf(shader_id, bxdf_parameters, local_wo, get_sampler()->generate_2d());
				auto world_wi = frame.local_to_world(local_wi);
				ray = make_ray(shadow_ray_origin, world_wi);
				pdf_bsdf = pdf;
				auto w = ite(pdf > 0.f, 1.f / pdf, 0.f);
				beta *= w * brdf;
				first_intersection.albedo = ite(depth==0, bxdf_parameters.base_color, first_intersection.albedo);
			}
		};
	};

	$if(first_intersection.valid())
	{
		// Write g_buffer after temporal denoising, as the g_buffer is used for temporal reprojection
		if (denoiser_ext)
			pixel_radiance = denoiser_ext->temporal_filter(pixel_coord, first_intersection, pixel_radiance, g_buffer);
		g_buffer.write(pixel_coord, first_intersection);
	}$else{
		g_buffer.set_default(pixel_coord);
		pixel_radiance = BackgroundColor;
	};
	return pixel_radiance;
}


};