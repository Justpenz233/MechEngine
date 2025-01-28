//
// Created by MarvelLi on 2025/1/20.
//

#include "wireframe_pass.h"
#include "Render/Core/math_function.h"
#include "Render/Core/RayCastHit.h"
#include "Render/Core/shape.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
namespace MechEngine::Rendering
{
void wireframe_pass::CompileShader(Device& device, bool bDebugInfo)
{
	WireFrameShader = luisa::make_unique<Shader2D<>>(device.compile<2>(
		[&]() noexcept {
		auto pixel = dispatch_id().xy();
		// Fetch intersection from GBuffer
	}));
}
void wireframe_pass::PostPass(CommandList& command_list) const
{
	RenderPass::PostPass(command_list);

}
Float wireframe_pass::wireframe_intensity
(const ray_intersection& intersection, const Float2& pixel_pos) const
{
	// Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	// Currently, we only draw the first intersection with the wireframe, which means
	// the wireframe would disappear in refractive or alpha blended surface
	//@see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	//@see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	auto wireframe_intensity = def(0.f);
	$if(intersection.shape->is_mesh())
	{
		auto material_data = Scene.GetMaterialProxy()->get_material_data(intersection.material_id);
		$if(material_data->show_wireframe == 1)
		{
			auto view = Scene.GetCameraProxy()->get_main_view();
			auto vertex_data = Scene.GetStaticMeshProxy()->get_vertices(
				intersection.shape->mesh_id,
				intersection.primitive_id);
			auto transform = Scene.get_instance_transform(
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
}