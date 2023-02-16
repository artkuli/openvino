// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <vector>
#include "ngraph_conversion_tests/plugin_specific_ngraph_conversion.hpp"

using namespace NGraphConversionTestsDefinitions;

namespace {

INSTANTIATE_TEST_CASE_P(smoke_Basic, PluginSpecificConversion, ::testing::Values("CPU"), PluginSpecificConversion::getTestCaseName);

}  // namespace
