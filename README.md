# Machine Learning-based Multi-Input Switching (MIS) Support in OpenSTA

## Overview

This project extends [OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA) to support **Multi-Input Switching (MIS)** timing analysis using machine learning. 

By default, OpenSTA only supports **Single Input Switching (SIS)**. This modification enables dynamic delay annotation for MIS conditions by hooking a trained ML model into the STA flow — *no core rebuilding needed for adding or updating models.*


sta -version = 2.6.0

---

## 🛠️ Build Instructions

No changes to the build system are required. This version builds exactly like vanilla OpenSTA. Please refer to the [OpenSTA build instructions](https://github.com/The-OpenROAD-Project/OpenSTA#build-instructions) for details.

---

## 📦 Dependencies

All dependencies remain the same as the original OpenSTA project.

---

## 🔧 Configuration

A new configuration file `models_config.json` is introduced to define the ML models used for MIS delay prediction.

### Example:

```json
{
  "lazyLoad": true,
  "models": [
    {
      "name": "nand2x1_sc",
      "path": "nand2x1_model.json",
      "modify_annotation": true,
      "inputFormat": ["load", "slew_a", "slew_b", "skew_ab"],
      "outputFormat": ["rise_delay", "rise_slew"],
      "scaleInput": [
        7e-16, 4.65e-14,
        5e-12, 3.2e-10,
        5e-12, 3.2e-10,
        -1e-9, 1e-9
      ],
      "scaleOutput": [
        7.59e-13, 2.96e-10,
        4.56e-12, 5.34e-10
      ]
    }
  ]
}
```

### Key Points:
- `modify_annotation: true` enables delay override for that gate.
- Input/output formats and scaling ranges align with your ML model's expectations.
- You can add models for additional gates by simply modifying the JSON — no recompilation needed.

---

## 🧩 Source Code Changes

The following new classes are introduced under the `modification/` directory:

- `StaInterface`: Interface layer between STA engine and ML models.
- `MlModel`: Handles loading and querying the model JSONs.
- `DataToModel`: Extracts input features from OpenSTA data.

Additionally, a new **public method** is added to the `Graph` class:

```cpp
void Graph::changeArrivals(Vertex* vertex, Arrival* new_arrivals);
```

This enables controlled modification of arrival times for specific gates during analysis.

### Integration Point:
The hook for model-based annotation is invoked from:

```cpp
ArrivalVisitor::visit()  // Located in search/Search.cc
```

## Approach

Assumption: the ML model modifies the annotation (arrival) of the fanin pins (input pins) of each gate to capture MIS effects on the output pin. We integrate this within the SIS flow of OpenSTA by hijacking the fanin arrival before computing the output arrival.

1. **ArrivalVisitor Hook**  
   In `search/Search.cc`, the `ArrivalVisitor::visit` method drives the STA graph traversal. We insert our modification just before OpenSTA computes the output-pin arrivals by calling `hackModelUpdate` in `modification/StaInterface.cc`.

2. **Identify Output Pin**  
   In `StaInterface::hackModelUpdate(Vertex*)`, we check if the current vertex corresponds to an output pin (`direction->isOutput()`) and has fanout. If so, and an ML model is available for its cell, we proceed.

3. **Gather Fanin Data**  
   `updateAnnotation_fanin_from_fanin(DataToModel*)` builds a `DataToModel` object containing:
   - The output vertex `Zn`
   - Fanin vertices A and B
   - Original arrival arrays (`getAnnotationArray`)
   - Slew information (`getSlew`)
   - Load capacitance on the output pin (`getLoadCapacitance`)

4. **ML Inference**  
   `MlModel::Modify(DataToModel*)` takes the original fanin arrivals and slews, constructs two input vectors (one for A→Y, one for B→Y, swapping A/B in the “slew” and “skew” features), invokes the FDeep model, and decodes the scaled outputs back to physical arrival/slew values.

5. **Apply Modified Arrivals**  
   Back in `updateAnnotation_fanin_from_fanin`, we write the modified arrival arrays into the graph via a new public `Graph::changeArrivals` helper (a friend of `Vertex`), updating A and B’s arrival annotations.

6. **Resume SIS Flow**  
   With fanin arrivals modified, OpenSTA’s existing combinational traversal computes the output-pin arrivals (for Y) including the ML-captured MIS effect.

**Assumptions**  
- ML-model configuration is per-cell (per-model) in `models_config.json`.  
- If a model is missing or `modify_annotation` is false, we leave original SIS arrivals untouched.  
- We only modify fanin arrivals, then let the standard OpenSTA path computation propagate to the output.
- the Modify part of the code could be wrong, but there will be an easy fix for that

---




