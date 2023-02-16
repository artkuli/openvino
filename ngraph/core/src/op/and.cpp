// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/op/and.hpp"
#include "itt.hpp"
#include "ngraph/runtime/host_tensor.hpp"
#include "ngraph/runtime/reference/and.hpp"

using namespace std;
using namespace ngraph;

NGRAPH_RTTI_DEFINITION(op::v1::LogicalAnd, "LogicalAnd", 1, util::BinaryElementwiseLogical);

op::v1::LogicalAnd::LogicalAnd(const Output<Node>& arg0,
                               const Output<Node>& arg1,
                               const AutoBroadcastSpec& auto_broadcast)
    : BinaryElementwiseLogical(arg0, arg1, auto_broadcast)
{
    constructor_validate_and_infer_types();
}

bool op::v1::LogicalAnd::visit_attributes(AttributeVisitor& visitor)
{
    NGRAPH_OP_SCOPE(v1_LogicalAnd_visit_attributes);
    BinaryElementwiseLogical::visit_attributes(visitor);
    return true;
}

shared_ptr<Node> op::v1::LogicalAnd::clone_with_new_inputs(const OutputVector& new_args) const
{
    NGRAPH_OP_SCOPE(v1_LogicalAnd_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<v1::LogicalAnd>(new_args.at(0), new_args.at(1), this->get_autob());
}

namespace logand
{
    template <element::Type_t ET>
    bool evaluate(const HostTensorPtr& arg0,
                  const HostTensorPtr& arg1,
                  const HostTensorPtr& out,
                  const op::AutoBroadcastSpec& broadcast_spec)
    {
        runtime::reference::logical_and(arg0->get_data_ptr<ET>(),
                                        arg1->get_data_ptr<ET>(),
                                        out->get_data_ptr<ET>(),
                                        arg0->get_shape(),
                                        arg1->get_shape(),
                                        broadcast_spec);
        return true;
    }

    bool evaluate_logand(const HostTensorPtr& arg0,
                         const HostTensorPtr& arg1,
                         const HostTensorPtr& out,
                         const op::AutoBroadcastSpec& broadcast_spec)
    {
        bool rc = true;
        out->set_broadcast(broadcast_spec, arg0, arg1);
        switch (arg0->get_element_type())
        {
            NGRAPH_TYPE_CASE(evaluate_logand, boolean, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, i32, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, i64, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, u32, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, u64, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, f16, arg0, arg1, out, broadcast_spec);
            NGRAPH_TYPE_CASE(evaluate_logand, f32, arg0, arg1, out, broadcast_spec);
        default: rc = false; break;
        }
        return rc;
    }
} // namespace logand

bool op::v1::LogicalAnd::evaluate(const HostTensorVector& outputs,
                                  const HostTensorVector& inputs) const
{
    NGRAPH_OP_SCOPE(v1_LogicalAnd_evaluate);
    return logand::evaluate_logand(inputs[0], inputs[1], outputs[0], get_autob());
}

bool op::v1::LogicalAnd::has_evaluate() const
{
    NGRAPH_OP_SCOPE(v1_LogicalAnd_has_evaluate);
    switch (get_input_element_type(0))
    {
    case ngraph::element::boolean:
    case ngraph::element::i32:
    case ngraph::element::i64:
    case ngraph::element::u32:
    case ngraph::element::u64:
    case ngraph::element::f16:
    case ngraph::element::f32: return true;
    default: break;
    }
    return false;
}
