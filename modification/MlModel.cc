#include "MlModel.hh"
#include <fstream>
#include <iostream>
#include <stdexcept>

MlModel::MlModel() 
    : lazyLoad_(true) {
    try {
        loadConfigurations("/pkg/git/OpenSTA/modification/models_config.json");
    } catch (const std::exception &e) {
        std::cerr << "Error loading configurations: " << e.what() << std::endl;
    }
}

MlModel& MlModel::getInstance() {
    static MlModel instance;
    return instance;
}

void MlModel::loadConfigurations(const std::string &configFile) {
    std::ifstream file(configFile);
    if (!file) {
        throw std::runtime_error("Could not open configuration file: " + configFile);
    }
    json j;
    file >> j;

    if (j.contains("lazyLoad")) {
        lazyLoad_ = j["lazyLoad"].get<bool>();
    }

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

void MlModel::registerModel(const ModelConfig &config) {
    models_[config.name] = std::make_pair(config, nullptr);
    std::cout << "Registered model: " << config.name << " (path: " << config.modelPath << ")" << std::endl;
}

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

std::shared_ptr<fdeep::model> MlModel::getModel(const std::string &name) {
    auto it = models_.find(name);
    if (it == models_.end()) {
        throw std::runtime_error("Model not registered: " + name);
    }
    if (it->second.second != nullptr) {
        return it->second.second;
    }
    if (lazyLoad_) {
        it->second.second = load_model(it->second.first);
        return it->second.second;
    }
    throw std::runtime_error("Model " + name + " is not loaded and lazy loading is disabled.");
}

float MlModel::predict(const std::string &modelName, const std::vector<float> &input_data) {
    auto mdl = getModel(modelName);
    const auto result = mdl->predict({ fdeep::tensor(fdeep::tensor_shape(input_data.size()), input_data) });
    std::vector<float> vec = result.data()->to_vector();
    if (!vec.empty()) {
        return vec[0];
    } else {
        throw std::runtime_error("Model prediction returned an empty result.");
    }
}


std::pair<float,float> MlModel::calculateSkew(std::vector<float*> annotation) {
    float minSkew, maxSkew;
    float minA = annotation[0][2]; // fall min for
    float maxA = annotation[0][0]; // fall max

    float minB = annotation[1][2];
    float maxB = annotation[1][0];
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


std::tuple<bool, bool, bool, float*, float*, float> 
MlModel::getModelAnnotation(const std::string &modelToUse,
                            const std::vector<float*>& annotations,
                            const float load_cap,
                            const std::vector<float*>& slew) {
    try {
        if (annotations.empty() || annotations[0] == nullptr) {
            std::cerr << "[Warning] No annotations provided." << std::endl;
            return std::make_tuple(false, false, false, nullptr, nullptr, -1.0f);
        }

        auto model_iter = models_.find(modelToUse);
        if (model_iter == models_.end()) {
            std::cerr << "[Warning] Model not registered: " << modelToUse << std::endl;
            return std::make_tuple(false, false, false, nullptr, nullptr, -1.0f);
        }

        getModel(modelToUse); // Will try to load the model if lazyLoad is true

        std::vector<float> input_data;
        const auto& format = model_iter->second.first.inputFormat;

        for (const auto& key : format) { // adding more input argument to the model should be added here
            if (key == "load")
                input_data.push_back(load_cap); // if calculation nedded make funtion
            else if (key == "slew_a")
                input_data.push_back(slew[0][1]);
            else if (key == "slew_b")
                input_data.push_back(slew[1][1]);
            else if (key == "skew_ab")
                input_data.push_back(calculateSkew(annotations).first);
            else {
                std::cerr << "[Warning] Unknown input key: " << key << std::endl;
                return std::make_tuple(false, false, false, nullptr, nullptr, -1.0f);
            }
        }

        float pred_value = predict(modelToUse, input_data); // This will throw if fail

        const auto& out_format = model_iter->second.first.outputFormat;
        bool change_anno = false, change_cap = false, change_slew = false;

        for (const auto& out : out_format) {
            if (out == "rise_delay") change_anno = true;
            if (out == "rise_slew") change_slew = true;
        }

        // -1 to all the index of the array no new value is calculated
        float* new_anno = new float[4]{-1, -1, -1, -1};
        float* new_slew = new float[2]{-1, -1};

        if (change_anno) { // some array manupulation for more complex output
            new_anno[0] = pred_value;
        }

        if (change_slew) {
            new_slew[0] = pred_value;
        }

        return std::make_tuple(change_anno, change_slew, change_cap, new_anno, new_slew, -1.0f);
    } catch (const std::exception &e) {
        std::cerr << "[Warning] getModelAnnotation failed: " << e.what() << std::endl;
        return std::make_tuple(false, false, false, nullptr, nullptr, -1.0f);
    }
}

