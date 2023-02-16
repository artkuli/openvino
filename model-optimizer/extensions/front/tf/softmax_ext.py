# Copyright (C) 2018-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

from mo.front.extractor import FrontExtractorOp
from mo.ops.softmax import Softmax


class SoftmaxExtractor(FrontExtractorOp):
    op = 'Softmax'
    enabled = True

    @classmethod
    def extract(cls, node):
        # the default value for the TF Softmax is -1
        axis = -1
        if 'axis' in node.pb.attr:
            axis = node.pb.attr['axis'].i
        Softmax.update_node_stat(node, {'axis': axis})
        return cls.enabled
