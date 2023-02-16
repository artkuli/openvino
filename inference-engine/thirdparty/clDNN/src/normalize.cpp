// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "normalize_inst.h"
#include "primitive_type_base.h"
#include "error_handler.h"
#include "json_object.h"
#include <string>

namespace cldnn {
primitive_type_id normalize::type_id() {
    static primitive_type_base<normalize> instance;
    return &instance;
}

layout normalize_inst::calc_output_layout(normalize_node const& node) {
    assert(static_cast<bool>(node.get_primitive()->output_data_type) == false &&
           "Output data type forcing is not supported for normalize_node!");
    auto input_node_layout = node.input().get_non_padded_output_layout();
    auto output_type = input_node_layout.data_type;

    if (node.has_fused_primitives()) {
        output_type = node.get_fused_output_layout().data_type;
    } else if (input_node_layout.data_type == data_types::u8 || input_node_layout.data_type == data_types::i8) {
        output_type = data_types::f32;
    }

    return layout(output_type, input_node_layout.format, input_node_layout.size);
}

std::string normalize_inst::to_string(normalize_node const& node) {
    auto node_info = node.desc_to_json();
    auto desc = node.get_primitive();
    auto epsilon = desc->epsilon;
    auto norm_region = desc->across_spatial ? "across spatial" : "within spatial";
    auto& input = node.input();
    auto& scale_input = node.scale();

    std::stringstream primitive_description;

    json_composite normalize_info;
    normalize_info.add("input id", input.id());
    normalize_info.add("scale input id", scale_input.id());
    normalize_info.add("epsilon", epsilon);
    normalize_info.add("normalization region", norm_region);

    node_info->add("normalize info", normalize_info);
    node_info->dump(primitive_description);

    return primitive_description.str();
}

normalize_inst::typed_primitive_inst(network_impl& network, normalize_node const& node) : parent(network, node) {
    /// Scale f dimension should be 1 (if all channels have the same scale) or equal to input feature size (one scale per channel).
    auto scale_layout = node.scale().get_output_layout();
    auto scale_size = scale_layout.size;
    auto scale_feature_size = scale_size.feature[0];
    auto input_layout = node.input().get_output_layout();
    auto input_feature_size = input_layout.size.feature[0];

    if (scale_feature_size != 1) {
        CLDNN_ERROR_NOT_EQUAL(node.id(),
                              "Scale feature size",
                              scale_feature_size,
                              "input feature size",
                              input_feature_size,
                              "");
    }

    // All other dimensions should be 1
    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "Scale input size elements count",
                          (int32_t)scale_size.count(),
                          "scale feature size",
                          scale_feature_size,
                          "Dimensions mismatch of scale input in Normalize layer!");
}
}  // namespace cldnn
