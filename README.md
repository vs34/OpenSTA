# Machine Learning-based Multi-Input Switching (MIS) Support in OpenSTA

## Overview

This project extends [OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA) to support **Multi-Input Switching (MIS)** timing analysis using machine learning. 

By default, OpenSTA only supports **Single Input Switching (SIS)**. This modification enables dynamic delay annotation for MIS conditions by hooking a trained ML model into the STA flow — *no core rebuilding needed for adding or updating models.*

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

---




