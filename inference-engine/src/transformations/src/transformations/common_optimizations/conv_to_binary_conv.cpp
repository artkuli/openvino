// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "transformations/common_optimizations/conv_to_binary_conv.hpp"
#include "itt.hpp"

#include <memory>
#include <vector>

#include <ngraph/opsets/opset5.hpp>
#include <ngraph/pattern/op/wrap_type.hpp>
#include <ngraph/rt_info.hpp>
#include <ngraph/validation_util.hpp>

NGRAPH_RTTI_DEFINITION(ngraph::pass::ConvToBinaryConv, "ConvToBinaryConv", 0);

static std::vector<uint8_t> binarize_weights(const std::vector<float>& weights) {
    std::vector<uint8_t> out;
    size_t bits_per_byte = 8;

    for (size_t i = 0; i < weights.size(); i += 8) {
        uint8_t val = 0;
        for (size_t j = 0; j < std::min(bits_per_byte, weights.size() - i); j++) {
            if (weights[i + j] == 1.0f)
                val |= 1 << j;
        }
        out.push_back(val);
    }
    return out;
}

ngraph::pass::ConvToBinaryConv::ConvToBinaryConv() {
    MATCHER_SCOPE(ConvToBinaryConv);
    auto fq_pattern = ngraph::pattern::wrap_type<opset5::FakeQuantize>(
            {ngraph::pattern::any_input(),
             ngraph::pattern::any_input(),
             ngraph::pattern::any_input(),
             ngraph::pattern::wrap_type<opset5::Constant>(),
             ngraph::pattern::wrap_type<opset5::Constant>()},
            pattern::consumers_count(1));
    auto conv_pattern = ngraph::pattern::wrap_type<opset5::Convolution>({fq_pattern, ngraph::pattern::wrap_type<opset5::Constant>()});

    ngraph::matcher_pass_callback callback = [=](pattern::Matcher &m) {
        auto conv = std::dynamic_pointer_cast<opset5::Convolution>(m.get_match_root());
        if (!conv)
            return false;
        auto fq = std::dynamic_pointer_cast<opset5::FakeQuantize>(conv->input_value(0).get_node_shared_ptr());
        if (!fq || fq->get_levels() != 2)
            return false;

        auto output_low_constant = std::dynamic_pointer_cast<opset5::Constant>(fq->input_value(3).get_node_shared_ptr());
        if (!output_low_constant)
            return false;
        auto output_low = output_low_constant->cast_vector<float>();
        bool output_low_is_zero = std::all_of(output_low.begin(), output_low.end(), [] (float f) -> bool { return f == 0.0f; });
        bool output_low_is_minus_one = std::all_of(output_low.begin(), output_low.end(), [] (float f) -> bool { return f == -1.0f; });
        auto output_high_constant = std::dynamic_pointer_cast<opset5::Constant>(fq->input_value(4).get_node_shared_ptr());
        if (!output_high_constant)
            return false;
        auto output_high = output_high_constant->cast_vector<float>();
        bool output_high_is_one = std::all_of(output_high.begin(), output_high.end(), [] (float f) -> bool { return f == 1.0f; });

        if (!(output_high_is_one && (output_low_is_zero || output_low_is_minus_one)))
                return false;

        auto weights_constant = std::dynamic_pointer_cast<opset5::Constant>(conv->input_value(1).get_node_shared_ptr());
        if (!weights_constant)
            return false;

        auto weights = weights_constant->cast_vector<float>();
        if (!std::all_of(weights.begin(), weights.end(), [] (float f) -> bool { return f == -1.0f || f == 1.0f; }))
            return false;

        auto bin_weights = binarize_weights(weights);
        auto bin_weights_constant = std::make_shared<opset5::Constant>(element::u1, weights_constant->get_shape(), bin_weights.data());

        if (output_low_is_zero && output_high_is_one) {
            auto new_conv = std::make_shared<opset5::BinaryConvolution>(conv->input_value(0), bin_weights_constant,
                                                                        conv->get_strides(),
                                                                        conv->get_pads_begin(),
                                                                        conv->get_pads_end(),
                                                                        conv->get_dilations(),
                                                                        opset5::BinaryConvolution::BinaryConvolutionMode::XNOR_POPCOUNT,
                                                                        -1,
                                                                        conv->get_auto_pad());
            new_conv->set_friendly_name(conv->get_friendly_name());
            std::vector<int64_t> axes;
            std::vector<int64_t> weights_reduced_shape = {-1};
            for (size_t i = 1; i < weights_constant->get_shape().size(); i++) {
                axes.push_back(i);
            }
            for (size_t i = 2; i < weights_constant->get_shape().size(); i++) {
                weights_reduced_shape.push_back(1);
            }
            auto weights_reduced = std::make_shared<opset5::ReduceSum>(
                    op::Constant::create(element::f32, weights_constant->get_shape(), weights),
                    op::Constant::create(element::i64, Shape{axes.size()}, axes), false);
            std::shared_ptr<Node> weights_reduced_reshaped = std::make_shared<opset5::Reshape>(weights_reduced,
                                                                              op::Constant::create(element::i64,
                                                                                                   Shape{weights_reduced_shape.size()},
                                                                                                   weights_reduced_shape),
                                                                              false);
            weights_reduced_reshaped = ngraph::get_constant_from_source(weights_reduced_reshaped);
            auto add = std::make_shared<opset5::Add>(new_conv, weights_reduced_reshaped);
            auto mul = std::make_shared<opset5::Multiply>(add, op::Constant::create(element::f32, Shape{}, {0.5}));
            copy_runtime_info(conv, {new_conv, add, mul});
            replace_node(conv, mul);

            return true;
        }

        auto new_conv = std::make_shared<opset5::BinaryConvolution>(conv->input_value(0), bin_weights_constant,
                                                                    conv->get_strides(),
                                                                    conv->get_pads_begin(),
                                                                    conv->get_pads_end(),
                                                                    conv->get_dilations(),
                                                                    opset5::BinaryConvolution::BinaryConvolutionMode::XNOR_POPCOUNT,
                                                                    0,
                                                                    conv->get_auto_pad());
        new_conv->set_friendly_name(conv->get_friendly_name());
        copy_runtime_info(conv, new_conv);
        replace_node(conv, new_conv);

        return true;
    };

    auto m = std::make_shared<ngraph::pattern::Matcher>(conv_pattern, matcher_name);
    this->register_matcher(m, callback);
}
