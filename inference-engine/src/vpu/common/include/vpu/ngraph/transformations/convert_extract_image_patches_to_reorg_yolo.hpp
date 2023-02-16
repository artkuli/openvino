// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <ngraph/pass/graph_rewrite.hpp>

namespace vpu {

class ConvertExtractImagePatchesToReorgYolo : public ngraph::pass::MatcherPass {
public:
    NGRAPH_RTTI_DECLARATION;
    ConvertExtractImagePatchesToReorgYolo();
};

}  // namespace vpu
