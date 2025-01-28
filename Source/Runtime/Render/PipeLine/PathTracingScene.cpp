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
#include "rasterizer/rasterizer.h"

namespace MechEngine::Rendering
{

void PathTracingScene::LoadRenderSettings()
{
	GpuScene::LoadRenderSettings();
	bUseRIS = GConfig.Get<bool>("PathTracing", "RIS");
	bUseSVGF = GConfig.Get<bool>("PathTracing", "SVGF");
}

void PathTracingScene::InitBuffers()
{
	GpuScene::InitBuffers();
}

void PathTracingScene::CompileShader()
{
	auto WindowSize = Window->framebuffer().size();

	if (bUseRIS)
		reservoirs = device.create_buffer<ris_reservoir>(WindowSize.x * WindowSize.y);

	if (bUseSVGF)
	{
		svgf = make_unique<class svgf>(WindowSize, frame_buffer());
		svgf->CompileShader(device, bShaderDebugInfo);
	}

	GpuScene::CompileShader();
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
	stream << CmdList.commit();

	CmdList << (*MainShader)(FrameCounter++, TimeCounter++).dispatch(GetWindosSize());
	if (bUseSVGF) svgf->PostPass(CmdList);
	stream << CmdList.commit();

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
			g_buffer.write(pixel_coord, intersection);
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
				auto occluded = has_hit(make_ray(x, light_sample.w_i, 0.01f, 0.99f));
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
			}
		};
	};
	$if(first_intersection.valid())
	{
		g_buffer.write(pixel_coord, first_intersection);
		if (bUseSVGF)
		{
			pixel_radiance = svgf->temporal_filter(pixel_coord, first_intersection, pixel_radiance);
		}
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
				view->world_to_pixel((transform * make_float4(vertex_data[0]->position(), 1.f)).xyz()),
				view->world_to_pixel((transform * make_float4(vertex_data[1]->position(), 1.f)).xyz()),
				view->world_to_pixel((transform * make_float4(vertex_data[2]->position(), 1.f)).xyz()));

			//@see https://backend.orbit.dtu.dk/ws/portalfiles/portal/3735323/wire-sccg.pdf
			wireframe_intensity = exp2(-2.f * square(d)); // I = exp2(-2 * d^2)
		};
	};
	return wireframe_intensity;
}

};