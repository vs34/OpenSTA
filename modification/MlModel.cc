#include "MlModel.h"

MlModel::MlModel() {
    // Load ML model here (if required)
}

MlModel& MlModel::getInstance() {
    static MlModel instance;
    return instance;
}

float MlModel::predict(float* annotation) {
    // Example: Simple transformation (replace with actual ML logic)
    return *annotation * 1.1f; // Scaling the value
}

