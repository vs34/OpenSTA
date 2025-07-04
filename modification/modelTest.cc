// load_and_predict.cpp
//
// Standalone example that loads a frugally‑deep model from a hard‑coded JSON file,
// uses a hard‑coded input vector, runs a forward pass, and prints the outputs.
// No external prompts—everything is in code.

#include <iostream>
#include <vector>
#include <fdeep/fdeep.hpp>

float minMaxScale(float x, float minVal, float maxVal) {
    return (x - minVal) / (maxVal - minVal);
}


float inverseMinMaxScale(float s, float minVal, float maxVal) {
    return s * (maxVal - minVal) + minVal;
}


int main()
{
    // 1) Hard‑coded path to your frugally‑deep JSON model
    const std::string model_path = "nand2x1_model.json";

    // 2) Load the model using fdeep::load_model
    std::shared_ptr<fdeep::model> model;
    try
    {
        model = std::make_shared<fdeep::model>(fdeep::load_model(model_path));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading fdeep model: " << e.what() << "\n";
        return 1;
    }

    // 3) Hard‑coded input vector (4 features)
    std::vector<float> input_data = {
        7e-16f,  // example feature 1
        5e-12f,  // example feature 2
        5e-12f,  // example feature 3
       -1e-9f    // example feature 4
    };

    input_data[0] = minMaxScale(input_data[0],7e-16, 4.65e-14);
    input_data[1] = minMaxScale(input_data[1],5e-12, 3.2e-10);
    input_data[2] = minMaxScale(input_data[2],5e-12, 3.2e-10);
    input_data[3] = minMaxScale(input_data[3],-1e-9, 1e-9);
    // 4) Wrap inputs into an fdeep::tensor
    const auto shape  = fdeep::tensor_shape(input_data.size());
    const auto tensor = fdeep::tensor(shape, input_data);

    // 5) Run prediction
    std::vector<fdeep::tensor> result;
    try
    {
        result = model->predict({ tensor });
    }
    catch (const std::exception& e)
    {
        std::cerr << "Prediction error: " << e.what() << "\n";
        return 2;
    }

    // 6) Print outputs
    std::cout << "Model output:";
    for (const auto& out_tensor : result)
    {
        for (float v : out_tensor.to_vector())
        {
            std::cout << "1 " << inverseMinMaxScale(v,7.59e-13, 2.96e-10) << '\n';
            std::cout << "2 " << inverseMinMaxScale(v,4.56e-12, 5.34e-10) << '\n';
        }
    }
    std::cout << std::endl;

    return 0;
}

