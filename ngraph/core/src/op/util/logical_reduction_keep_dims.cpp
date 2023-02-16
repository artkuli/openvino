// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/op/util/logical_reduction_keep_dims.hpp"
#include "itt.hpp"
#include "ngraph/attribute_visitor.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/validation_util.hpp"

using namespace std;
using namespace ngraph;

NGRAPH_RTTI_DEFINITION(op::util::LogicalReductionKeepDims, "LogicalReductionKeepDims", 1);

op::util::LogicalReductionKeepDims::LogicalReductionKeepDims(
    const ngraph::Output<ngraph::Node>& arg,
    const ngraph::Output<ngraph::Node>& reduction_axes,
    const bool keep_dims)
    : LogicalReduction(arg, reduction_axes)
    , m_keep_dims{keep_dims}
{
}

bool ngraph::op::util::LogicalReductionKeepDims::visit_attributes(AttributeVisitor& visitor)
{
    NGRAPH_OP_SCOPE(v0_util_LogicalReductionKeepDims_visit_attributes);
    visitor.on_attribute("keep_dims", m_keep_dims);
    return true;
}

void op::util::LogicalReductionKeepDims::validate_and_infer_types()
{
    NGRAPH_OP_SCOPE(v0_util_LogicalReductionKeepDims_validate_and_infer_types);
    if (m_keep_dims)
    {
        const auto input_shape = get_input_partial_shape(0);
        const auto input_rank = input_shape.rank();
        PartialShape result_shape{PartialShape::dynamic(input_rank)};

        set_input_is_relevant_to_shape(1);
        set_output_type(0, get_input_element_type(0), result_shape);

        if (input_shape.is_dynamic())
            return;

        if (auto axes_const = get_constant_from_source(input_value(1)))
        {
            AxisSet reduction_axes;
            auto reduction_axes_val = axes_const->cast_vector<int64_t>();
            for (auto axis : reduction_axes_val)
            {
                try
                {
                    axis = normalize_axis(this, axis, input_rank);
                }
                catch (const ngraph_error&)
                {
                    NODE_VALIDATION_CHECK(this,
                                          false,
                                          "Reduction axis (",
                                          axis,
                                          ") is out of bounds ",
                                          "(argument shape: ",
                                          input_shape,
                                          ", reduction axes: ",
                                          reduction_axes,
                                          ")");
                }
                reduction_axes.insert(axis);
            }

            std::vector<Dimension> dims;
            for (int64_t i = 0; i < input_rank.get_length(); i++)
            {
                if (reduction_axes.count(i) == 0)
                {
                    dims.push_back(input_shape[i]);
                }
                else
                {
                    dims.emplace_back(Dimension{1});
                }
            }
            result_shape = PartialShape(dims);
        }

        set_output_type(0, get_input_element_type(0), result_shape);
    }
    else
    {
        LogicalReduction::validate_and_infer_types();
    }
}
