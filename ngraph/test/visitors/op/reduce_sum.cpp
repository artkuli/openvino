// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "reduce_ops.hpp"

using Type = ::testing::Types<ngraph::op::v1::ReduceSum>;
INSTANTIATE_TYPED_TEST_CASE_P(attributes_reduce_sum, ReduceOpsAttrTest, Type);
