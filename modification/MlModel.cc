#include "MlModel.hh"
#include <fdeep/fdeep.hpp>
#include <vector>

MlModel::MlModel() : model_(fdeep::load_model("/pkg/git/OpenSTA/modification/model.json")) {
        // Constructor body (if needed)
}
MlModel& MlModel::getInstance() {
    static MlModel instance;
    return instance;
}

float MlModel::predict(float* annotation) {
    // Define input vector
    std::vector<float> input_data = {
        (annotation[1] - 7e-16) / (4.598836e-14 - 7e-16),
        (annotation[0] - 5e-12) / (3.199877e-10 - 5e-12),
        (annotation[0] - 5e-12) / (3.199877e-10 - 5e-12),
        (2.50e-10 + 2.5e-10) / 5e-10
    };
    
    // Run prediction
    const auto result = model_.predict({fdeep::tensor(fdeep::tensor_shape(static_cast<std::size_t>(input_data.size())), input_data)});
    std::vector<float> vec = result.data()->to_vector();
    
    // Choose the output based on annotation[2] (rfi value)
    return annotation[2] == 1 ? vec[0] * 3.061052e-10 : vec[1] * 4.423355e-10;
}

