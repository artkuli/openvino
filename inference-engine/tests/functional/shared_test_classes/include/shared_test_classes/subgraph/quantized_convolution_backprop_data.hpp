// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <memory>

#include "shared_test_classes/base/layer_test_utils.hpp"
#include "ngraph_functions/builders.hpp"
#include "ngraph_functions/utils/ngraph_helpers.hpp"

namespace SubgraphTestsDefinitions {

typedef std::tuple<
        InferenceEngine::SizeVector,
        InferenceEngine::SizeVector,
        std::vector<ptrdiff_t>,
        std::vector<ptrdiff_t>,
        InferenceEngine::SizeVector,
        size_t,
        ngraph::op::PadType,
        size_t,
        ngraph::helpers::QuantizationGranularity> quantConvBackpropDataSpecificParams;
typedef std::tuple<
        quantConvBackpropDataSpecificParams,
        InferenceEngine::Precision,
        InferenceEngine::SizeVector,
        LayerTestsUtils::TargetDevice> quantConvBackpropDataLayerTestParamsSet;

class QuantConvBackpropDataLayerTest : public testing::WithParamInterface<quantConvBackpropDataLayerTestParamsSet>,
                                            virtual public LayerTestsUtils::LayerTestsCommon {
public:
    static std::string getTestCaseName(testing::TestParamInfo<quantConvBackpropDataLayerTestParamsSet> obj);

protected:
    void SetUp() override;
};

}  // namespace SubgraphTestsDefinitions