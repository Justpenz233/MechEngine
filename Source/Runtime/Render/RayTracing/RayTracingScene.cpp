//
// Created by MarvelLi on 2024/3/27.
//

#include "RayTracingScene.h"
#include "Render/Core/TypeConvertion.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/VertexData.h"
#include "Render/ViewportInterface.h"
#include "Render/Core/bxdf_context.h"
#include "Render/Core/draw_lines.h"
#include "Render/material/shading_function.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"

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
	g_buffer.base_color = device.create_image<float>(PixelStorage::R11G11B10,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.normal = device.create_image<float>(PixelStorage::R11G11B10,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.depth = device.create_image<float>(PixelStorage::FLOAT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.instance_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.material_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.frame_buffer = &Window->framebuffer();
	LOG_INFO("Init render frame buffer: {} {}",Window->framebuffer().size().x, Window->framebuffer().size().y);
	Viewport = InViewport;

	RayTracingScene::CompileShader();
}

void RayTracingScene::AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform)
{
	StaticMeshProxy->AddStaticMesh(InMesh, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateStaticMesh(StaticMeshComponent* InMesh)
{
	StaticMeshProxy->UpdateStaticMesh(InMesh);
}

void RayTracingScene::EraseMesh(StaticMeshComponent* InMesh)
{

}

void RayTracingScene::AddMaterial(Material* InMaterial)
{
	MaterialProxy->TryAddMaterial(InMaterial);
}

void RayTracingScene::UpdateMaterial(Material* InMaterial)
{
	MaterialProxy->UpdateMaterial(InMaterial);
}

void RayTracingScene::AddCamera(CameraComponent* InCamera, TransformComponent* InTransform)
{
	CameraProxy->AddCamera(InCamera, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateCamera(CameraComponent* InCamera)
{
	CameraProxy->UpdateCamera(InCamera);
}

void RayTracingScene::UpdateTransform(TransformComponent* InTransform)
{
	TransformProxy->UpdateTransform(InTransform);
}

void RayTracingScene::AddLight(LightComponent* InLight, TransformComponent* InTransform)
{
	LightProxy->AddLight(InLight, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateLight(LightComponent* InLight)
{
	LightProxy->UpdateLight(InLight);
}

Matrix4d RayTracingScene::GetViewMatrix()
{
	return CameraProxy->GetViewMatrix();
}

Matrix4d RayTracingScene::GetProjectionMatrix()
{
	return CameraProxy->GetProjectionMatrix();
}

void RayTracingScene::UploadData()
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

	view_data_buffer = CameraProxy->GetCurrentViewData();
	stream << rtAccel.build() << synchronize();
}

void RayTracingScene::Render()
{
	ASSERTMSG(g_buffer.frame_buffer, "Frame buffer is not initialized");
	stream << (*MainShader)(view_data_buffer, LightProxy->LightCount()).dispatch(GetWindosSize());

	if(ViewMode != ViewMode::FrameBuffer)
		stream << (*ViewModePass)(static_cast<uint>(ViewMode)).dispatch(GetWindosSize());

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

void RayTracingScene::CompileShader()
{
	// Compile base shaders
	GPUSceneInterface::CompileShader();

	// Main pass shader
	MainShader = luisa::make_unique<Shader2D<view_data, uint>>(device.compile<2>(
			[&](Var<view_data> view, UInt LightCount) noexcept {
			// Calc view space cordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
			auto pixel_coord = dispatch_id().xy();
			auto pixel       = make_float2(pixel_coord) + .5f;
			auto resolution  = make_float2(view.viewport_size);
			auto p           = (pixel * 2.0f - resolution) / resolution;
			p *= make_float2(1.f, -1.f);

			auto ray = CameraProxy->GenerateRay(p);
			auto intersection = intersect(ray, view);
			intersection.pixel_coord = pixel_coord; // Ugly, to do someting

			Float3 pixel_color = make_float3(1.f);
			$if(intersection.valid())
			{
				auto material_data = MaterialProxy->get_material_data(intersection.material_id);
				g_buffer.instance_id->write(pixel_coord, make_uint4(intersection.instace_id));
				g_buffer.material_id->write(pixel_coord, make_uint4(intersection.material_id));
				/************************************************************************
				 *								Shading
				 ************************************************************************/
				// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
				auto x      = intersection.position_world;
				auto normal = intersection.vertex_normal_world;
				auto w_o    = -ray->direction();
				auto reflect_dir = reflect(-w_o, normal);

				pixel_color = make_float3(0.f);
				$for(light_id, LightCount) {
					// First calculate light color, as rendering equation is L_i(x, w_i)
					auto light_data = LightProxy->get_light_data(light_id);
					auto light_transform = TransformProxy->get_transform_data(light_data.transform_id);
					auto light_dir = normalize(light_transform->get_location() - x);
					auto calc_lighting = [&](const Float3& w_i) {
						Float3 light_color = make_float3(0.f);

						$if(dot(w_i, normal) >= 0.f)
						{
							// Dispatch light evaluate polymorphically, so that we can have different light type
							LightProxy->light_virtual_call.dispatch(
								light_data.light_type, [&](const light_base* light) {
									light_color = light->l_i(light_data, light_transform.transformMatrix, x, w_i);
								});
						};

						Float3 mesh_color = make_float3(0.f);
						$if(dot(w_o, normal) >= 0.f)
						{
							// calculate mesh color
							bxdf_context context{
								.g_buffer = g_buffer,
								.ray = ray,
								.intersection = intersection,
								.w_o = w_o,
								.w_i = w_i,
								.material_data = MaterialProxy->get_material_data(intersection.material_id)
							};
							MaterialProxy->material_virtual_call.dispatch(
								material_data.material_type, [&](const material_base* material) {
									material->fill_g_buffer(context);
									mesh_color = material->bxdf(context);
								});
						};
						// combine light and mesh color
						return mesh_color * light_color * dot(w_i, intersection.vertex_normal_world);
					};

					// Only two sample, one is light dir, one is reflect dir
					// pixel_color += calc_lighting(light_dir) * 0.5f + calc_lighting(reflect_dir) * 0.5f;
					pixel_color += calc_lighting(light_dir);
				};

				$if(material_data->show_wireframe == 1)
				{
					//@see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
					//@see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
					pixel_color = select(pixel_color, make_float3(0.f, 0.f, 0.f),
						IsWireFrame(view, pixel,
							intersection.vertex_ndc[0],
							intersection.vertex_ndc[1],
							intersection.vertex_ndc[2], 0.5f));
				};
			}
			$else
			{
				g_buffer.set_default(pixel_coord, make_float4(1.f));
			};
			frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(pixel_color), 1.f));
		}));
}
}
