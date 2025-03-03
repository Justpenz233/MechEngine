//
// Created by MarvelLi on 2025/3/3.
//

#include "denoiser_ext.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{
denoiser_ext::denoiser_ext(class GpuScene* InScene) : scene(InScene)
{}
void denoiser_ext::CompileShader(luisa::compute::Device& Device, bool bDebugInfo)
{
	auto resolution = scene->GetWindosSize();
	albedo = Device.create_buffer<float4>(resolution.x * resolution.y);
	normal = Device.create_buffer<float4>(resolution.x * resolution.y);
	noisy_image = Device.create_buffer<float4>(resolution.x * resolution.y);
	output_image = Device.create_buffer<float4>(resolution.x * resolution.y);
	auto denoiser_ext = Device.extension<DenoiserExt>();
	denoiser = denoiser_ext->create(scene->GetStream());
	{
		auto input = DenoiserExt::DenoiserInput{ resolution.x, resolution.y };
		input.push_noisy_image(noisy_image.view(), output_image.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
		input.push_feature_image("albedo", albedo.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
		input.push_feature_image("normal", normal.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
		input.noisy_features = false;
		input.filter_quality = DenoiserExt::FilterQuality::FAST;
		input.prefilter_mode = DenoiserExt::PrefilterMode::NONE;
		denoiser->init(input);
	}

	copy_frame_buffer_shader = luisa::make_unique<Shader2D<>>(Device.compile<2>(
		[&]() noexcept {
			auto pixel_coord = dispatch_id().xy();
			auto color = scene->frame_buffer()->read(pixel_coord);

			auto index = pixel_coord.x + pixel_coord.y * resolution.x;
			noisy_image->write(index, make_float4(color.xyz(), 1.f));
		}, { .enable_debug_info = bDebugInfo, .name = "CopyFrameBufferShader" }));

	write_frame_buffer_shader = luisa::make_unique<Shader2D<>>(Device.compile<2>(
		[&]() noexcept {
			auto pixel_coord = dispatch_id().xy();
			auto index = pixel_coord.x + pixel_coord.y * resolution.x;
			auto color = output_image->read(index);
			scene->frame_buffer()->write(pixel_coord, make_float4(color.xyz(), 1.f));
		}, { .enable_debug_info = bDebugInfo, .name = "WriteFrameBufferShader" }));
}

void denoiser_ext::PostPass(luisa::compute::CommandList& command_list) const
{
	RenderPass::PostPass(command_list);
	auto& g_buffer = scene->get_gbuffer();
	command_list
	<< g_buffer.albedo.copy_to(albedo)
	<< g_buffer.normal.copy_to(normal)
	<< (*copy_frame_buffer_shader)().dispatch(scene->GetWindosSize());
	scene->get_stream() << command_list.commit();
	denoiser->execute(true);
	scene->get_stream() << (*write_frame_buffer_shader)().dispatch(scene->GetWindosSize());
}

} // namespace MechEngine::Rendering