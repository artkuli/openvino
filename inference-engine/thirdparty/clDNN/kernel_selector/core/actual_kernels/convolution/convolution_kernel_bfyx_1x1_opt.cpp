﻿// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "convolution_kernel_bfyx_1x1_opt.h"
#include <vector>

namespace kernel_selector {

convolution_kernel_bfyx_1x1_opt::convolution_kernel_bfyx_1x1_opt()
    : ConvolutionKernelBase("convolution_gpu_bfyx_1x1_opt") {}

ParamsKey convolution_kernel_bfyx_1x1_opt::GetSupportedKey() const {
    ParamsKey k;
    k.EnableInputDataType(Datatype::F32);
    k.EnableInputWeightsType(WeightsType::F32);
    k.EnableOutputDataType(Datatype::F32);
    k.EnableInputLayout(DataLayout::bfyx);
    k.EnableOutputLayout(DataLayout::bfyx);
    k.EnableTensorOffset();
    k.EnableTensorPitches();
    k.EnableSubGroup();
    k.EnableBiasPerFeature();
    k.EnableBiasPerOutput();
    k.EnableNonBiasTerm();
    k.EnableBatching();
    return k;
}

struct block_params {
    int32_t out_width;
    int32_t out_height;
    int32_t out_depth;
};

static block_params get_out_block_size(const convolution_params& p) {
    auto out_depth = 8;

    if (p.output.X().v == 7) {
        auto gws0 = p.output.X().v / 7;
        auto gws1 = p.output.Y().v / 1;
        auto gws2 = 2 * (p.output.Feature().v * p.output.Batch().v) / 8;  // process 8 output channels per Workitem

        auto compute_units = p.engineInfo.computeUnitsCount;
        auto total_threads = (gws0 * gws1 * gws2) / 64;
        if (total_threads < compute_units) {
            out_depth /= 2;
            total_threads *= 2;
        }
        if (total_threads < compute_units) {
            out_depth /= 2;
            total_threads *= 2;
        }
        return {7, 1, out_depth};
    } else if (p.output.X().v == 14) {
        return {7, 1, 8};
    } else if (p.output.X().v == 28) {
        return {7, 2, 4};
    } else if (p.output.X().v == 56) {
        return {8, 1, 8};
    }

    return {1, 1, 1};
}

ConvolutionKernelBase::DispatchData convolution_kernel_bfyx_1x1_opt::SetDefault(const convolution_params& cp,
                                                                                int) const {
    DispatchData dispatchData = ConvolutionKernelBase::SetDefault(cp);

    constexpr size_t sub_group_size = 8;

    auto block = get_out_block_size(cp);

    dispatchData.gws[0] = cp.output.X().v / block.out_width;
    dispatchData.gws[1] = cp.output.Y().v / block.out_height;
    // process 8 output channels per Workitem
    dispatchData.gws[2] = 2 * (cp.output.Feature().v * cp.output.Batch().v) / block.out_depth;

    dispatchData.lws[0] = 1;
    dispatchData.lws[1] = 1;
    dispatchData.lws[2] = 2 * sub_group_size;

    return dispatchData;
}

KernelsPriority convolution_kernel_bfyx_1x1_opt::GetKernelsPriority(const Params& /*params*/, const optional_params& /*options*/) const {
    return FORCE_PRIORITY_1;
}

bool convolution_kernel_bfyx_1x1_opt::Validate(const Params& p, const optional_params& o) const {
    if (!ConvolutionKernelBase::Validate(p, o)) {
        return false;
    }
    const convolution_params& cp = static_cast<const convolution_params&>(p);

    if (cp.stride.x != 1 || cp.stride.y != 1)
        return false;

    if (cp.filterSize.x != 1 || cp.filterSize.y != 1)
        return false;

    if (cp.output.Feature().v % 64 != 0)
        return false;

    if (cp.padding.x != 0 || cp.padding.y != 0)
        return false;

    // if block sizes are 1x1, then this algorithm is probably not the best
    auto block = get_out_block_size(cp);
    if (block.out_width == 1 && block.out_height == 1)
        return false;

    if (cp.output.X().v % block.out_width != 0)
        return false;
    if (cp.output.Y().v % block.out_height != 0)
        return false;

    return true;
}

JitConstants convolution_kernel_bfyx_1x1_opt::GetJitConstants(const convolution_params& params,
                                                              const DispatchData& dispatchData) const {
    auto jit = Parent::GetJitConstants(params, dispatchData);

    auto block = get_out_block_size(params);
    jit.AddConstant(MakeJitConstant("OUT_BLOCK_WIDTH", block.out_width));
    jit.AddConstant(MakeJitConstant("OUT_BLOCK_HEIGHT", block.out_height));
    jit.AddConstant(MakeJitConstant("OUT_BLOCK_DEPTH", block.out_depth));

    return jit;
}

WeightsLayout convolution_kernel_bfyx_1x1_opt::GetPreferredWeightsLayout(const convolution_params &cp) const {
    auto block = get_out_block_size(cp);
    if (block.out_depth == 8)
        return WeightsLayout::os_iyx_osv64;
    if (block.out_depth == 4)
        return WeightsLayout::os_iyx_osv32;
    if (block.out_depth == 2)
        return WeightsLayout::os_iyx_osv16;
    else
        return WeightsLayout::yxio;
}

KernelsData convolution_kernel_bfyx_1x1_opt::GetKernelsData(const Params& params,
                                                            const optional_params& options) const {
    KernelsData kd = GetCommonKernelsData(params, options);
    return kd;
}

}  // namespace kernel_selector
