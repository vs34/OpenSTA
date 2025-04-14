#include "MlModel.hh"
#include <fstream>
#include <iostream>
#include <stdexcept>

// Constructor: load configuration from external JSON file
MlModel::MlModel() 
    : lazyLoad_(true) // default lazy load true; may be overridden by config
{
    try {
        // Change the file path as needed.
        loadConfigurations("/pkg/git/OpenSTA/modification/models_config.json");
    } catch (const std::exception &e) {
        std::cerr << "Error loading configurations: " << e.what() << std::endl;
    }
}

// Singleton instance
MlModel& MlModel::getInstance() {
    static MlModel instance;
    return instance;
}

// Load configuration file using nlohmann/json and register each model.
void MlModel::loadConfigurations(const std::string &configFile) {
    std::ifstream file(configFile);
    if (!file) {
        throw std::runtime_error("Could not open configuration file: " + configFile);
    }
    json j;
    file >> j;

    // Global lazy load setting
    if (j.contains("lazyLoad")) {
        lazyLoad_ = j["lazyLoad"].get<bool>();
    }

    // Iterate over models array and register each configuration.
    if (j.contains("models") && j["models"].is_array()) {
        for (const auto &item : j["models"]) {
            ModelConfig config;
            config.name = item["name"].get<std::string>();
            config.modelPath = item["path"].get<std::string>();
            if (item.contains("inputFormat"))
                config.inputFormat = item["inputFormat"].get<std::vector<std::string>>();
            if (item.contains("outputFormat"))
                config.outputFormat = item["outputFormat"].get<std::vector<std::string>>();
            registerModel(config);
        }
    }
}

// Register a model by storing its configuration in the map.
void MlModel::registerModel(const ModelConfig &config) {
    // Insert the config and set the model pointer to nullptr initially.
    models_[config.name] = std::make_pair(config, nullptr);
    std::cout << "Registered model: " << config.name << " (path: " << config.modelPath << ")" << std::endl;
}

// Helper: load a model from disk given its configuration.
std::shared_ptr<fdeep::model> MlModel::load_model(const ModelConfig &config) {
    try {
        auto mdl = std::make_shared<fdeep::model>(fdeep::load_model(config.modelPath));
        std::cout << "Loaded model " << config.name << " from " << config.modelPath << std::endl;
        return mdl;
    } catch (const std::exception &e) {
        std::cerr << "Error loading model " << config.name << ": " << e.what() << std::endl;
        throw;
    }
}

// Get model pointer by name; if not loaded and lazyLoad_ is true, load it now.
std::shared_ptr<fdeep::model> MlModel::getModel(const std::string &name) {
    auto it = models_.find(name);
    if (it == models_.end()) {
        throw std::runtime_error("Model not registered: " + name);
    }
    // If model is already loaded, return it.
    if (it->second.second != nullptr) {
        return it->second.second;
    }
    // If not loaded and lazyLoad_ is enabled, load the model.
    if (lazyLoad_) {
        it->second.second = load_model(it->second.first);
        return it->second.second;
    }
    // Otherwise, throw an error or return nullptr.
    throw std::runtime_error("Model " + name + " is not loaded and lazy loading is disabled.");
}

// Predict using a specific model with given input data.
float MlModel::predict(const std::string &modelName, const std::vector<float> &input_data) {
    auto mdl = getModel(modelName);
    const auto result = mdl->predict({ fdeep::tensor(fdeep::tensor_shape(input_data.size()), input_data) });
    std::vector<float> vec = result.data()->to_vector();
    if (!vec.empty()) {
        return vec[0];  // Return the first output (adjust if needed)
    } else {
        throw std::runtime_error("Model prediction returned an empty result.");
    }
}


std::pair<float,float> MlModel::calculateSkew(std::vector<float*> annotation) {
    float minSkew, maxSkew;
    float minA = std::min(annotation[0][2] , annotation[0][3]);
    float maxA = std::max(annotation[0][0] , annotation[0][1]);

    float minB = std::min(annotation[1][2] , annotation[1][3]);
    float maxB = std::max(annotation[1][0] , annotation[1][1]);
    if(minA==minB){
        minSkew = 0;
        maxSkew = std::max(std::fabs(minA - maxB), std::fabs(minB - maxA));
    }
    else if(maxA<minB){
        minSkew = std::fabs(maxA - minB);
        maxSkew = std::max(std::fabs(minA - maxB), std::fabs(minB - maxA));
    }
    else if(maxB<minA){
        minSkew = std::fabs(maxB - minA);
        maxSkew = std::max(std::fabs(minA - maxB), std::fabs(minB - maxA));
    }
    else {
        minSkew = 0;
        maxSkew = std::max(std::fabs(minA - maxB), std::fabs(minB - maxA));
    }
    
    return std::make_pair(minSkew, maxSkew);
}

// For demonstration: get annotations from the specified model.
// This function returns a tuple with three booleans and three float pointers.
// You can change the tuple contents and logic as needed.
std::tuple<bool, bool, bool, float, float, float> 
MlModel::getModelAnnotation(const std::string &modelToUse,
                            const std::vector<float*>& annotations,
                            const std::vector<float>& load_cap,
                            const std::vector<float*>& slew)
{
    // If a model name is provided, load that model.
    if (!modelToUse.empty()) {
        getModel(modelToUse); // lazy-load happens here
    }

    // Check if there is at least one annotation.
    if (annotations.empty() || annotations[0] == nullptr) {
        std::cerr << "Error: No annotations provided." << std::endl;
        return std::make_tuple(false, false, false, 0.0f, 0.0f, 0.0f);
    }

    // For demonstration, use the first annotation pointer to make a prediction.
    // In a real-world scenario, you would format the annotations based on load_cap and slew.
    float* pred1 = new float;
    
    // Prepare an input vector from the first annotation.
    // Here we assume the annotation array has the required parameters.
    // You may need to adjust the scaling based on your configuration.
    // *pred1 = predict(modelToUse, { load_cap[0], slew[0][1], slew[1][0],calculateSkew(annotations)});
std::vector<float> input_data;
input_data.push_back(load_cap[0]);
input_data.push_back(slew[0][1]); // make sure slew[0] is valid and has 2+ elements
input_data.push_back(slew[1][0]); // same check for slew[1]
input_data.push_back((calculateSkew(annotations)).second);

*pred1 = predict(modelToUse, input_data);

    // Create dummy adjustments for the other predictions.
    
    // Return tuple with status flags (for example, model loaded, prediction valid, extra flag)
    return std::make_tuple(true, true, false, pred1[0], pred1[1], -1);
}
