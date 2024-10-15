//
// Created by MarvelLi on 2024/3/27.
//

#include "RayTracingScene.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/VertexData.h"
#include "Render/ViewportInterface.h"
#include "Render/Core/bxdf_context.h"
#include "Render/Core/math_function.h"
#include "Render/Core/random.h"
#include "Render/material/shading_function.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/sampler/independent_sampler.h"

namespace MechEngine::Rendering
{

RayTracingScene::RayTracingScene(Stream& stream, Device& device, ImGuiWindow* InWindow, ViewportInterface* InViewport) noexcept:
GPUSceneInterface(stream, device), Window(InWindow)
{
	CameraProxy = luisa::make_unique<CameraSceneProxy>(*this);
	LightProxy = luisa::make_unique<LightSceneProxy>(*this);
	TransformProxy = luisa::make_unique<TransformSceneProxy>(*this);
	StaticMeshProxy = luisa::make_unique<StaticMeshSceneProxy>(*this);
	MaterialProxy = luisa::make_unique<MaterialSceneProxy>(*this);
	LineProxy = luisa::make_unique<LineSceneProxy>(*this);
	ShapeProxy = luisa::make_unique<ShapeSceneProxy>(*this);
	auto size = Window->framebuffer().size();
	LOG_DEBUG("GBuffer initial size: {} {}", size.x, size.y);
	g_buffer.base_color = device.create_image<float>(PixelStorage::BYTE4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.normal = device.create_image<float>(PixelStorage::BYTE4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.depth = device.create_buffer<float>(size.x * size.y);
	g_buffer.instance_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.material_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.frame_buffer = &Window->framebuffer();
	LOG_INFO("Init render frame buffer: {} {}",Window->framebuffer().size().x, Window->framebuffer().size().y);
	Viewport = InViewport;

	RayTracingScene::CompileShader();
}

void RayTracingScene::UploadRenderData()
{
	auto UpdateBindlessArrayIfDirty = [&]() {
		if(bindlessArray.dirty())
		{
			stream << bindlessArray.update();
		}
	};

	// Make sure static mesh data is uploaded before transform data
	// Because need to allocate instance id from accel
	ShapeProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	StaticMeshProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	CameraProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	MaterialProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LightProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	// This may also update transform in accel
	TransformProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LineProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	if (rtAccel.dirty()) stream << rtAccel.build() << synchronize();
}

void RayTracingScene::Render()
{
	stream << (*MainShader)(FrameCounter++).dispatch(GetWindosSize());

	if(ViewMode != ViewMode::FrameBuffer) [[unlikely]]
		stream << (*ViewModePass)(static_cast<uint>(ViewMode)).dispatch(GetWindosSize());

	LineProxy->PostRenderPass(stream);

	stream << synchronize();
}

ImageView<float> RayTracingScene::frame_buffer() noexcept
{
	return Window->framebuffer();
}

uint2 RayTracingScene::GetWindosSize() const noexcept
{
	return Window->framebuffer().size();
}

Float3 RayTracingScene::render_pixel(Var<Ray> ray, const Float2& pixel_pos) const
{
	ray_intersection wireframe_intersection;
	Float3 pixel_radiance = make_float3(0.f);
	Float3 beta = make_float3(1.f);

	$for(depth, 0, 2)
	{
		auto intersection = intersect(ray);
		$if(depth == 0) {wireframe_intersection = intersection;}; // For wireframe pass
		$if(intersection.shape->is_light() & depth == 0)
		{
			auto light_data = LightProxy->get_light_data(intersection.shape.light_id);
			pixel_radiance = light_data.light_color;
			$break;
		}
		$elif (intersection.shape->is_surface()) // Surface shade
		{
			auto material_data = MaterialProxy->get_material_data(intersection.material_id);

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
					auto light_sample = light_type->sample_li(light, x, normal, get_sampler()->generate_2d());
					auto cos = dot(normalize(light_sample.w_i), normal);
					$if(cos > 0.01f & !has_hit(make_ray(shadow_ray_origin, light_sample.w_i, 0.01f, 0.99f)))
					{
						MaterialProxy->shader_call.dispatch(
							material_data.shader_id, [&](const shader_base* material) {
								pixel_radiance += beta * material->bxdf(bxdf_parameters, w_o, light_sample.w_i)
									* light_sample.l_i / light_sample.pdf * cos;
							});
					};
				});
			// Sample brdf
			MaterialProxy->shader_call.dispatch(
			material_data.shader_id, [&](const shader_base* material) {
				auto [wi, pdf] = material->sample(get_sampler()->generate_2d());
				wi = orthogonal_basis(normal, wi);
				beta *= material->bxdf(bxdf_parameters, w_o, wi) * max(dot(wi, normal), 0.f) / pdf;
				ray = make_ray(shadow_ray_origin, wi);
			});
		};
	};

	// Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	// Currently, we only draw the first intersection with the wireframe, which means
	// the wireframe would disappear in refractive or alpha blended surface
	//@see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	//@see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	$if(wireframe_intersection.shape->is_surface())
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
			pixel_radiance = lerp(pixel_radiance, make_float3(0.f), wireframe_intensity);
		};
	};
	return pixel_radiance;
}

void RayTracingScene::render_main_view(const UInt& frame_index)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	get_sampler()->init(pixel_coord, frame_index);

	// Ray trace rasterization
	auto pixel_pos = make_float2(pixel_coord) + .5f;
	auto ray = view->generate_ray(pixel_pos);

	auto color = render_pixel(ray, pixel_pos);
	auto pre_color = srgb_to_linear(frame_buffer()->read(pixel_coord).xyz());
	auto now_color = select(pre_color + (color - pre_color) / Float(frame_index), color, frame_index == 0);

	frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(now_color), 1.f));
}

void RayTracingScene::CompileShader()
{
	LoadRenderSettings();

	// Initialize the sampler
	sampler = luisa::make_unique<independent_sampler>();

	// Compile base shaders
	GPUSceneInterface::CompileShader();

	// Main pass shader
	MainShader = luisa::make_unique<Shader2D<uint>>(device.compile<2>(
		[&](UInt frame_index) noexcept { render_main_view(frame_index); }));

}
}
