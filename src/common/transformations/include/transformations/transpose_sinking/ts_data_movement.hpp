// Copyright (C) 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/pass/graph_rewrite.hpp"
#include "openvino/pass/pass.hpp"
#include "transformations_visibility.hpp"

namespace ov {
namespace pass {
namespace transpose_sinking {

class TRANSFORMATIONS_API TSDataMovementForward;
class TRANSFORMATIONS_API TSDataMovementBackward;

}  // namespace transpose_sinking
}  // namespace pass
}  // namespace ov

/**
 * @ingroup ie_transformation_common_api
 * @brief TSDataMovementForward transformation sinks Transpose through BatchToSpace, SpaceToBatch
 * and Pad operations in the forward direction.
 * These operations are categorized as "DataMovement" and are handled in a similar way in this transformation.
 */
class ov::pass::transpose_sinking::TSDataMovementForward : public ov::pass::MatcherPass {
public:
    OPENVINO_RTTI("ov::pass::TSDataMovementForward", "0");
    TSDataMovementForward();
};

/**
 * @ingroup ie_transformation_common_api
 * @brief TSDataMovementBackward transformation sinks Transpose through BatchToSpace, SpaceToBatch
 * and Pad operations in the backward direction.
 * These operations are categorized as "DataMovement" and are handled in a similar way in this transformation.
 */
class ov::pass::transpose_sinking::TSDataMovementBackward : public ov::pass::MatcherPass {
public:
    OPENVINO_RTTI("ov::pass::TSDataMovementBackward", "0");
    TSDataMovementBackward();
};
