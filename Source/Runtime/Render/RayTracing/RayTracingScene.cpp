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
	CompileShader();
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

	stream << rtAccel.build() << synchronize();
}

void RayTracingScene::Render()
{
	ASSERTMSG(g_buffer.frame_buffer, "Frame buffer is not initialized");
	stream << (*MainShader)(LightProxy->LightCount()).dispatch(GetWindosSize());

	if(ViewMode != ViewMode::FrameBuffer)
		stream << (*ViewModeShader)(static_cast<uint>(ViewMode)).dispatch(GetWindosSize());

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
	MainShader = luisa::make_unique<Shader2D<uint>>(device.compile<2>(
			[&](UInt LightCount) noexcept {
			// Calc view space cordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
			auto pixel_coord = dispatch_id().xy();
			auto pixel       = make_float2(pixel_coord) + .5f;
			auto Resolution  = make_float2(GetWindosSize());
			auto p           = (pixel * 2.0f - Resolution) / Resolution;
			p *= make_float2(1.f, -1.f);

			auto ray = CameraProxy->GenerateRay(p);
			auto intersection = intersect(ray); intersection.pixel_coord = pixel_coord;
			auto x = intersection.position_world;
			auto normal = intersection.vertex_normal_world;
			auto w_o = -ray->direction();

			$if(intersection.valid())
			{
				Float3 color = make_float3(0.f);
				// Rendering equation : L_o(x, w_0) = L_e(x, w_0) + \int_{\Omega} bxdf(x, w_i, w_0) L_i(x, w_i) (n \cdot w_i) dw_i
				$for(light_id, LightCount)
				{
					// First calculate light color, as rendering equation is L_i(x, w_i)
					auto light_data = LightProxy->get_light_data(light_id);
					auto light_transform = TransformProxy->get_transform_data(light_data.transform_id);
					auto light_dir = normalize(light_transform->get_location() - x);

					auto w_i = light_dir;
					Float3 light_color = make_float3(0.f);

					$if(dot(light_dir, normal) > 0.f)
					{
						// Dispatch light evaluate polymorphically, so that we can have different light type
						LightProxy->light_virtual_call.dispatch(light_data.light_type,
						[&](const light_base* light) {
							light_color = light->l_i(light_data, light_transform.transformMatrix, x, w_i);
						});
					};

					// calculate mesh color
					auto material_data = MaterialProxy->get_material_data(intersection.material_id);
					g_buffer.instance_id->write(pixel_coord, make_uint4(intersection.instace_id));
					g_buffer.material_id->write(pixel_coord, make_uint4(intersection.material_id));

					bxdf_context context{
						.ray = ray, .intersection = intersection, .w_o = w_o, .w_i = w_i,
						.material_data = MaterialProxy->get_material_data(intersection.material_id),
						.g_buffer = g_buffer
					};
					Float3 mesh_color;
					MaterialProxy->material_virtual_call.dispatch(material_data.material_type,
					[&](const material_base* material) {
						material->fill_g_buffer(context);
						mesh_color = material->bxdf(context);
					});

					// combine light and mesh color
					color += mesh_color * light_color * dot(w_i, intersection.vertex_normal_world);
				};
				frame_buffer()->write(pixel_coord, make_float4(gamma_correct(color), 1.f));
			}
			$else
			{
				g_buffer.fill_color(pixel_coord, make_float4(1.f));
				frame_buffer()->write(pixel_coord, make_float4(1.f));
			};
		}));

	ViewModeShader = luisa::make_unique<Shader2D<uint>>(device.compile<2>([&](UInt ViewMode)
	{
		auto pixel_coord = dispatch_id().xy();
		$switch(ViewMode)
		{
			// $case(static_cast<uint>(ViewMode::DepthBuffer))
			// {
			// 	frame_buffer()->write(pixel_coord, g_buffer_view.depth->read(pixel_coord));
			// };
			$case(static_cast<uint>(ViewMode::NormalWorldBuffer))
			{
				frame_buffer()->write(pixel_coord, g_buffer.normal->read(pixel_coord));
			};
			$case(static_cast<uint>(ViewMode::BaseColorBuffer))
			{
				frame_buffer()->write(pixel_coord, g_buffer.base_color->read(pixel_coord));
			};
		};
	}));
}
}
