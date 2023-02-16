﻿// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <iostream>
#include "quantize_kernel_base.h"
#include "kernel_selector_utils.h"
#include <string>

namespace kernel_selector {

bool QuantizeKernelBase::Validate(const Params& p, const optional_params&) const {
    const quantize_params& params = static_cast<const quantize_params&>(p);
    if (params.inputs.size() != 5)
        return false;

    // Binary packed output is possible only with bfyx input and b_fs_yx_32fp output
    if (params.output.GetDType() == Datatype::BINARY &&
        (params.output.GetLayout() != DataLayout::b_fs_yx_32fp || params.inputs[0].GetLayout() != DataLayout::bfyx))
        return false;

    return true;
}

JitConstants QuantizeKernelBase::GetJitConstants(const quantize_params& params, const CommonDispatchData& dispatchData) const {
    JitConstants jit = MakeBaseParamsJitConstants(params);

    if (params.packed_binary_output) {
        jit.AddConstant(MakeJitConstant("PACKED_BINARY_OUTPUT", params.packed_binary_output));
        jit.AddConstant(MakeJitConstant("OUTPUT_FEATURE_NUM_PACKED", CeilDiv(params.output.Feature().v, 32)));
        jit.AddConstant(MakeJitConstant("OC_BLOCK_SIZE", 32));
        if ((params.inputs[3].LogicalSize() == 1 && params.inputs[4].LogicalSize() == 1) ||
            (params.inputs[3].LogicalSize() == params.inputs[3].Batch().v &&
             params.inputs[4].LogicalSize() == params.inputs[4].Batch().v)) {
            jit.AddConstant(MakeJitConstant("SINGLE_OUT_VAL", 1));

        } else if (params.inputs[3].LogicalSize() == params.output.Feature().v &&
                   params.inputs[4].LogicalSize() == params.output.Feature().v) {
            jit.AddConstant(MakeJitConstant("PER_CHANNEL_OUT_VAL", 1));
        } else {
            throw std::runtime_error("Unsupported const blob shape in node " + params.layerID);
        }
    }

    jit.AddConstant(MakeJitConstant("LEVELS", static_cast<float>(params.levels)));

    jit.AddConstant(MakeJitConstant("LWS_0", dispatchData.lws[0]));
    jit.AddConstant(MakeJitConstant("LWS_1", dispatchData.lws[1]));
    jit.AddConstant(MakeJitConstant("LWS_2", dispatchData.lws[2]));

    return jit;
}

KernelsData QuantizeKernelBase::GetKernelsData(const Params& params, const optional_params& options) const {
    assert(params.GetType() == KernelType::QUANTIZE);

    KernelData kd = KernelData::Default<quantize_params>(params);
    quantize_params& newParams = *static_cast<quantize_params*>(kd.params.get());

    if (!Validate(params, options)) {
        return {};
    }

    auto dispatchData = SetDefault(newParams, options);
    auto entry_point = GetEntryPoint(kernelName, newParams.layerID, options);
    auto cldnn_jit = GetJitConstants(newParams, dispatchData);
    auto jit = CreateJit(kernelName, cldnn_jit, entry_point);

    auto& kernel = kd.kernels[0];

    kernel.workGroups.global = dispatchData.gws;
    kernel.workGroups.local = dispatchData.lws;
    kernel.kernelString = GetKernelString(kernelName, jit, entry_point, params.engineInfo, DEFAULT);
    kernel.arguments = GetArgsDesc(static_cast<int>(newParams.inputs.size()), false, false);

    return {kd};
}
}  // namespace kernel_selector
