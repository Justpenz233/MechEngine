//
// Created by MarvelLi on 2025/10/2.
//

#include "denoiser.h"
#include "Render/Core/ray_intersection.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{

denoiser::denoiser(GpuScene* InScene)
{
	scene = InScene;
	WinSize = scene->GetWindosSize();

}

Bool denoiser::is_reproject_valid(const UInt2& pixel_coord, const UInt2& pre_coord, const ray_intersection& intersection, const geometry_buffer& g_buffer) const
{
	const float NORMAL_TOLERANCE = 5.0e-2;
	const float DEPTH_TOLERANCE = 5.0e-2;
	Bool is_valid = false;
	$if(all(pre_coord > make_uint2(0, 0)) & all(pre_coord < WinSize))
	{
		auto pre_instance = g_buffer.instance_id->read(pre_coord).x;
		auto pre_normal = g_buffer.normal->read(pre_coord).xyz();
		is_valid = (pre_instance == intersection.instance_id) & distance_squared(pre_normal, intersection.corner_normal_world) < NORMAL_TOLERANCE;
		auto pre_depth = g_buffer.read_depth(pre_coord);
		is_valid = is_valid & abs(pre_depth - intersection.depth) < DEPTH_TOLERANCE;
	};

	return is_valid;
}

// @see https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl#L93
Float3 denoiser::temporal_filter(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color, const geometry_buffer& g_buffer) const
{
	const float MAX_HISTORY_LENGTH = 32;
	Float2		pre_pixel_pos = intersection.motion_vector;
	UInt2		pre_pixel_coord = floor(pre_pixel_pos);
	auto  pixel_pos_frac = pre_pixel_pos - make_float2(pre_pixel_coord);
	auto  inv_pixel_pos_frac = 1.f - pixel_pos_frac;
	Float weights[4];
	weights[0] = inv_pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[1] = pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[2] = inv_pixel_pos_frac.x * pixel_pos_frac.y;
	weights[3] = pixel_pos_frac.x * pixel_pos_frac.y;

	Float3 sum_val = make_float3(0.f);
	Float  sum_spp = 0.f;
	Float  sum_weight = 0.f;
	Bool has_vaild_history = false;
	for (int dy = 0; dy <= 0; dy += 1)
	{
		for (int dx = 0; dx <= 0; dx += 1)
		{
			auto p = make_uint2(dx, dy) + pre_pixel_coord;
			$if(is_reproject_valid(pixel_coord, p, intersection, g_buffer))
			{
				has_vaild_history = true;
				auto pre_color = g_buffer.radiance->read(p);
				sum_weight += weights[dx + dy * 2];
				sum_val += pre_color.xyz() * weights[dx + dy * 2];
				sum_spp += history_length->read(p).x * weights[dx + dy * 2];
			};
		}
	}
	$if(has_vaild_history)
	{
		sum_val /= sum_weight;
		sum_spp /= sum_weight;
	};
	auto new_color = pixel_color;
	$if (has_vaild_history)
	{
		auto historyLength = min(sum_spp + 1.0f, MAX_HISTORY_LENGTH);
		new_color = ite(has_vaild_history, lerp(sum_val, pixel_color, 1.0f / historyLength), pixel_color);
		history_length->write(pixel_coord, make_uint4(UInt(historyLength)));
	}
	$else
	{
		history_length->write(pixel_coord, make_uint4(0u));
	};

	return new_color;
}
void denoiser::InitPass(Device& Device, luisa::compute::CommandList& command_list)
{
	RenderPass::InitPass(Device, command_list);
	history_length = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);
	static auto ClearHistoryLength = Device.compile<2>(
	[=]() noexcept {
		auto pixel_coord = dispatch_id().xy();
		history_length->write(pixel_coord, make_uint4(0u));
	}, { .enable_debug_info = false, .name = "ClearHistoryLength" });
	command_list << ClearHistoryLength().dispatch(WinSize.x, WinSize.y);
}
void denoiser::CompileShader(Device& Device, bool bDebugInfo)
{
	auto resolution = scene->GetWindosSize();

	if (bUseOIDN)
	{
		auto denoiserext = Device.extension<DenoiserExt>();
		if (denoiserext) {
			albedo = Device.create_buffer<float4>(resolution.x * resolution.y);
			normal = Device.create_buffer<float4>(resolution.x * resolution.y);
			noisy_image = Device.create_buffer<float4>(resolution.x * resolution.y);
			output_image = Device.create_buffer<float4>(resolution.x * resolution.y);
			denoiser_ext = denoiserext->create(scene->GetStream());
			{
				auto input = DenoiserExt::DenoiserInput{ resolution.x, resolution.y };
				input.push_noisy_image(noisy_image.view(), output_image.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
				input.push_feature_image("albedo", albedo.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
				input.push_feature_image("normal", normal.view(), DenoiserExt::ImageFormat::FLOAT3, DenoiserExt::ImageColorSpace::HDR);
				input.noisy_features = false;
				input.filter_quality = DenoiserExt::FilterQuality::FAST;
				input.prefilter_mode = DenoiserExt::PrefilterMode::NONE;
				denoiser_ext->init(input);
			}
		}

		copy_frame_buffer_shader = luisa::make_unique<Shader2D<>>(Device.compile<2>(
			[&]() noexcept {
				auto pixel_coord = dispatch_id().xy();
				auto color = scene->get_gbuffer().radiance->read(pixel_coord);
				auto index = pixel_coord.x + pixel_coord.y * resolution.x;
				noisy_image->write(index, make_float4(color.xyz(), 1.f));
			}, { .enable_debug_info = bDebugInfo, .name = "CopyFrameBufferShader" }));

		write_frame_buffer_shader = luisa::make_unique<Shader2D<>>(Device.compile<2>(
			[&]() noexcept {
				auto pixel_coord = dispatch_id().xy();
				auto index = pixel_coord.x + pixel_coord.y * resolution.x;
				auto color = output_image->read(index);
				scene->get_gbuffer().radiance->write(pixel_coord, make_float4(color.xyz(), 1.f));
				scene->frame_buffer()->write(pixel_coord, make_float4(color.xyz(), 1.f));
			}, { .enable_debug_info = bDebugInfo, .name = "WriteFrameBufferShader" }));
	}
}
void denoiser::PostPass(luisa::compute::CommandList& command_list) const
{
	if (bUseOIDN && denoiser_ext) {
		RenderPass::PostPass(command_list);
		auto& g_buffer = scene->get_gbuffer();
		command_list
		<< g_buffer.albedo.copy_to(albedo)
		<< g_buffer.normal.copy_to(normal)
		<< (*copy_frame_buffer_shader)().dispatch(scene->GetWindosSize());
		scene->get_stream() << command_list.commit();
		denoiser_ext->execute(true);
		scene->get_stream() << (*write_frame_buffer_shader)().dispatch(scene->GetWindosSize());
	}
}
} // namespace MechEngine::Rendering