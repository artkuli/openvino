# Introduction to the nGraph {#openvino_docs_nGraph_DG_Introduction}

The Inference Engine integrates the nGraph to represent a model in run time underneath of the conventional
`CNNNetwork` API, which is an instance of `ngraph::Function`.

Besides the representation update, nGraph supports new features:

1. nGraph contains several [sets of operations which are called `opset1`, `opset2` and etc.](../ops/opset.md).
Operations from these operation sets are generated by the Model Optimizer and are accepted in the Inference Engine.

2. Operation version is attached to each operation rather than to the entire IR file format.
IR is still versioned but has a different meaning. For details, see [Deep Learning Network Intermediate Representation and Operation Sets in OpenVINO™](../MO_DG/IR_and_opsets.md).

3. Creating models at runtime without loading IR from an xml/binary file. You can enable it by creating
`ngraph::Function` passing it to `CNNNetwork`.

4. Runtime reshape capability and constant folding are implemented through the nGraph code for more operations compared to previous releases.
As a result, more models can be reshaped. For details, see the [dedicated guide about the reshape capability](../IE_DG/ShapeInference.md).

5. Loading [model from ONNX format](../IE_DG/ONNX_Support.md) without converting it to the Inference Engine IR.

6. nGraph representation supports dynamic shapes. You can use `CNNNetwork::reshape()` method in order to specialize input shapes.

A complete picture of the existing flow is shown below.

![](img/TopLevelNGraphFlow.png)

## Read the Intermediate Representation to `CNNNetwork`

The IR version 10 automatically triggers the nGraph flow inside the Inference Engine.
When such IR is read in an application, the Inference Engine IR reader produces `CNNNetwork` that encapsulates the `ngraph::Function` instance underneath.

Interpretation of the IR version 10 differs from the old IR version.
Besides having a different operations set, the IR version 10 ignores the shapes and data types assigned to the ports in an XML file.
Both shapes and types are reinferred while loading to the Inference Engine using the nGraph shape and type propagation function that is a part of each nGraph operation.

## Build a Model in the Application

Alternative method to feed the Inference Engine with a model is to create the model in the run time.
It is achieved by creation of the `ngraph::Function` construction using nGraph operation classes and optionally user-defined operations.
For details, see [Add Custom nGraph Operations](../IE_DG/Extensibility_DG/AddingNGraphOps.md) and [examples](build_function.md).
At this stage, the code is completely independent of the rest of the Inference Engine code and can be built separately.
After you construct an instance of `ngraph::Function`, you can use it to create `CNNNetwork` by passing it to the new constructor for this class.

## See Also
- [Available Operations Sets](../ops/opset.md)