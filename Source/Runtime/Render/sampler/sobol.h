//
// Created by Mayn on 2024/11/2.
//

#pragma once

//
// Created by Mike Smith on 2022/2/9.
//

#include "sampler_base.h"
#include "Render/Core/sample.h"
#include "luisa/dsl/sugar.h"
namespace MechEngine::Rendering {

// using luisa::compute::UInt;
// using luisa::compute::UInt2;
// using luisa::uint;
// using luisa::compute::Expr;
// using luisa::compute::Float;
// using luisa::compute::Float2;
// using luisa::compute::Float3;
// using luisa::compute::Float4;
// using luisa::compute::Buffer;
// using luisa::compute::ULong;
// using luisa::ulong;
// using luisa::uint4;
using namespace luisa;
using namespace luisa::compute;

// Sobol Matrix Declarations




class ENGINE_API sobol_sampler : public sampler_base {
private:
    uint2 _resolution;
    uint _scale{};
    luisa::optional<UInt2> _pixel;
    luisa::optional<UInt> _dimension;
    luisa::optional<ULong> _sobol_index;
    BufferView<uint> _sobol_matrices;
    BufferView<ulong> _vdc_sobol_matrices;
    BufferView<ulong> _vdc_sobol_matrices_inv;
	#include "sobolmatrices.h"
public:
	sobol_sampler(GpuScene* Scene, Stream& stream) noexcept
		: sampler_base(Scene, stream)
	{
		_sobol_matrices = Scene->RegisterBuffer<uint>(SobolMatrixSize * NSobolDimensions);
		_vdc_sobol_matrices = Scene->RegisterBuffer<ulong>(SobolMatrixSize);
		_vdc_sobol_matrices_inv = Scene->RegisterBuffer<ulong>(SobolMatrixSize);
		stream << _sobol_matrices.copy_from(SobolMatrices32);
		auto res = Scene->GetWindosSize();
		_scale = next_pow2(std::max(res.x, res.y));
		auto							   m = std::bit_width(_scale) - 1u;
		std::array<ulong, SobolMatrixSize> vdc_sobol_matrices{};
		std::array<ulong, SobolMatrixSize> vdc_sobol_matrices_inv{};
		for (auto i = 0u; i < SobolMatrixSize; i++)
		{
			vdc_sobol_matrices[i] = VdCSobolMatrices[m - 1u][i];
			vdc_sobol_matrices_inv[i] = VdCSobolMatricesInv[m - 1u][i];
		}
		stream << _vdc_sobol_matrices.copy_from(vdc_sobol_matrices.data())
			   << _vdc_sobol_matrices_inv.copy_from(vdc_sobol_matrices_inv.data())
			   << commit();
	}

private:
    [[nodiscard]] static auto _fast_owen_scramble(Expr<uint> seed, UInt v) noexcept {
        v = reverse(v);
        v ^= v * 0x3d20adeau;
        v += seed;
        v *= (seed >> 16u) | 1u;
        v ^= v * 0x05526c56u;
        v ^= v * 0x53a22864u;
        return reverse(v);
    }

    template<bool scramble>
    [[nodiscard]] auto _sobol_sample(ULong a, Expr<uint> dimension, Expr<uint> hash) const noexcept {
        static Callable impl = [](ULong a, UInt dimension, BufferVar<uint> sobol_matrices, UInt hash) noexcept {
            auto v = def(0u);
            auto i = def(dimension * SobolMatrixSize);
            $while (a != 0ull) {
                v = ite((a & 1ull) != 0ull, v ^ sobol_matrices.read(i), v);
                a = a >> 1ull;
                i = i + 1u;
            };
            if constexpr (scramble) { v = _fast_owen_scramble(hash, v); }
            return v * 0x1p-32f;
        };
        return impl(a, dimension, _sobol_matrices, hash);
    }

    [[nodiscard]] auto _sobol_interval_to_index(uint m, UInt frame, Expr<uint2> p) const noexcept {
        if (m == 0u) { return cast<ulong>(frame); }
        static Callable impl = [](UInt m, UInt frame, UInt2 p, BufferVar<ulong> vdc, BufferVar<ulong> vdc_inv) noexcept {
            auto c = def(0u);
            auto m2 = m << 1u;
            auto index = cast<ulong>(frame) << cast<ulong>(m2);
            auto delta = def<ulong>(0ull);
            $while (frame != 0u) {
                $if ((frame & 1u) != 0u) {
                    auto v = vdc.read(c);
                    delta = delta ^ v;
                };
                frame >>= 1u;
                c += 1u;
            };
            // flipped b
            auto b = delta ^ ((cast<ulong>(p.x) << m) | cast<ulong>(p.y));
            auto d = def(0u);
            $while (b != 0ull) {
                $if ((b & 1ull) != 0ull) {
                    auto v = vdc_inv.read(d);
                    index = index ^ v;
                };
                b = b >> 1ull;
                d += 1u;
            };
            return index;
        };
        return impl(m, frame, p, _vdc_sobol_matrices, _vdc_sobol_matrices_inv);
    }

public:
	void init(Expr<uint2> pixel, Expr<uint> sample_index) noexcept override {
        _dimension.emplace(2u);
        _sobol_index.emplace(_sobol_interval_to_index(
            std::bit_width(_scale) - 1u, sample_index, pixel));
        _pixel.emplace(pixel);
    }

    [[nodiscard]] Float generate_1d() noexcept override {
        *_dimension = ite(*_dimension >= NSobolDimensions, 2u, *_dimension);
        auto hash = xxhash32(make_uint2(*_dimension, get_seed()));
        auto u = _sobol_sample<true>(*_sobol_index, *_dimension, hash);
        *_dimension += 1u;
        return clamp(u, 0.f, one_minus_epsilon);
    }
    [[nodiscard]] Float2 generate_2d() noexcept override {
        *_dimension = ite(*_dimension + 1u >= NSobolDimensions, 2u, *_dimension);
        auto hx = xxhash32(make_uint2(*_dimension, get_seed()));
        auto hy = xxhash32(make_uint2(*_dimension + 1u, get_seed()));
        auto ux = _sobol_sample<true>(*_sobol_index, *_dimension, hx);
        auto uy = _sobol_sample<true>(*_sobol_index, *_dimension + 1u, hy);
        *_dimension += 2u;
        return clamp(make_float2(ux, uy), 0.f, one_minus_epsilon);
    }
    [[nodiscard]] Float2 generate_pixel_2d() noexcept {
        auto ux = _sobol_sample<false>(*_sobol_index, 0u, 0u);
        auto uy = _sobol_sample<false>(*_sobol_index, 1u, 0u);
        auto s = static_cast<float>(_scale);
        return clamp(make_float2(ux, uy) * s - make_float2(*_pixel),
                     0.f, one_minus_epsilon);
    }
};
};