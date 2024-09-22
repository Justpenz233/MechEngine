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
	stream << (*MainShader)(++FrameCounter).dispatch(GetWindosSize());

	if(ViewMode != ViewMode::FrameBuffer)
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

std::pair<Float3, Float> RayTracingScene::calc_surface_point_color(
	Var<Ray> ray, const ray_intersection& intersection, bool global_illumination)
{
	auto material_data = MaterialProxy->get_material_data(intersection.material_id);

	/************************************************************************
	 *								Shading
	 ************************************************************************/
	auto x = intersection.position_world;
	auto w_o = normalize(-ray->direction());

	// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
	bxdf_context context{
		.intersection = intersection,
		.material_data = MaterialProxy->get_material_data(intersection.material_id),
		.w_o = w_o,
	};

	material_parameters bxdf_parameters;
	MaterialProxy->shader_call.dispatch(
		material_data.material_type, [&](const shader_base* material) {
			bxdf_parameters = material->calc_material_parameters(context);
		});
	// g_buffer.write(pixel_coord, bxdf_parameters, intersection);

	auto normal = bxdf_parameters.normal;

	// Raytracing gem 2,  CHAPTER 4 HACKING THE SHADOW TERMINATOR
	auto offset_ray = [&]() {
		auto vps = StaticMeshProxy->get_vertices(intersection.mesh_id, intersection.primitive_id);
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
	auto shadow_ray_origin = bShadowRayOffset ? offset_ray() : x;

	Float3 pixel_radiance = make_float3(0.f);
	$for(light_id, 128)
	{
		// First calculate light color, as rendering equation is L_i(x, w_i)
		auto light_data = LightProxy->get_light_data(light_id);
		$if(!light_data->valid()) {$break;};
		auto light_transform = TransformProxy->get_transform_data(light_data.transform_id);
		auto light_dir = normalize(light_transform->get_location() - x);

		auto calc_lighting = [&](const Float3& w_i, bool calc_shadow) {
			Float3 light_color = make_float3(0.f);
			Float3 mesh_color = make_float3(0.f);
			Float3 light_visibility = make_float3(1.);
			$if(dot(w_i, normal) >= 0.f)
			{
				context.w_i = w_i;
				// Dispatch light evaluate polymorphic, so that we can have different light type

				// Shadow ray
				if (calc_shadow)
					light_visibility = select(make_float3(1.), make_float3(0.),
						has_hit(make_ray(shadow_ray_origin, w_i, 0.01f, 1.f)));

				LightProxy->light_virtual_call.dispatch(
					light_data.light_type, [&](const light_base* light) {
						light_color = light->l_i(light_data, light_transform.transformMatrix, x, w_i);
					});

				MaterialProxy->shader_call.dispatch(
					material_data.material_type, [&](const shader_base* material) {
						mesh_color = material->bxdf(context, bxdf_parameters);
					});
			};
			return mesh_color * light_color * max(dot(w_i, normal), 0.001f) * light_visibility;
		};

		auto lighting = calc_lighting(light_dir, bRenderShadow);

		// If pipeline not using global illumination, we sample a lighting by the normal direction to gain more realistic result
		// Cheaper method in the those machine not have hardware acceleration
		if (!bGlobalIllumination)
			lighting = lighting * 0.95f + calc_lighting(reflect(-w_o, normal), false) * 0.05f;
		pixel_radiance += lighting;
	};
	if(global_illumination)
	{
		$if(material_data.alpha == 1.f) // Only reflect for opaque surface
		{
			// Reflection by Orthonormal Basis
			// See https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#orthonormalbases
			auto reflect_dir = sample_uniform_hemisphere_surface(get_sampler()->generate_2d());
			auto onb = orthogonal_basis(normal);
			reflect_dir = normalize(reflect_dir.x * onb[0] + reflect_dir.y * onb[1] + reflect_dir.z * onb[2]);

			auto reflect_ray = make_ray(offset_ray_origin(x, normal), reflect_dir);
			auto reflect_intersection = intersect(reflect_ray);
			$if(reflect_intersection.valid())
			{
				auto [reflect_radiance, alpha]
				= calc_surface_point_color(reflect_ray, reflect_intersection, false);

				pixel_radiance = pixel_radiance * 0.95f + reflect_radiance * 0.05f;
			};
		};
	}
	return {pixel_radiance, material_data.alpha};
}

Float3 RayTracingScene::render_pixel(Var<Ray> ray, const Float2& pixel_pos)
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
	$if(wireframe_intersection.valid())
	{
		auto material_data = MaterialProxy->get_material_data(wireframe_intersection.material_id);
		$if(material_data->show_wireframe == 1)
		{
			auto view = CameraProxy->get_main_view();
			auto vertex_data = StaticMeshProxy->get_vertices(
				wireframe_intersection.mesh_id,
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

void RayTracingScene::render_main_view(const UInt& frame_index)
{
	auto view = CameraProxy->get_main_view();
	// Calc view space coordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
	auto pixel_coord = dispatch_id().xy();
	get_sampler()->init(pixel_coord, frame_index);

	// Fill with default
	g_buffer.set_default(pixel_coord, make_float4(1.f));
	// frame_buffer()->write(pixel_coord, make_float4(1.f));

	// Ray trace rasterization
	auto pixel_pos = make_float2(pixel_coord) + .5f;
	auto ray = view->generate_ray(pixel_pos);
	auto color = render_pixel(ray, pixel_pos);
	frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(color), 1.f));
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
