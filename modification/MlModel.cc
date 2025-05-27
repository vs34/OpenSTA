#include "MlModel.hh"
#include "DataToModel.hh"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// Static singleton pointer
MlModel* MlModel::instance_ = nullptr;

// Private constructor
MlModel::MlModel() {
    try {
        loadConfigurations("models_config.json");
    } catch (const std::exception& e) {
        std::cerr << "[Error] MlModel init failed: " << e.what() << std::endl;
    }
}

// Singleton accessor
MlModel* MlModel::getInstance() {
    if (!instance_) {
        instance_ = new MlModel();
    }
    return instance_;
}

void MlModel::loadConfigurations(const std::string &configFile) {
    std::ifstream file(configFile);
    if (!file) throw std::runtime_error("Cannot open config: " + configFile);
    json j; file >> j;

    if (j.contains("lazyLoad"))
        lazyLoad_ = j["lazyLoad"].get<bool>();

    if (j.contains("models") && j["models"].is_array()) {
        for (auto &item : j["models"]) {
            ModelConfig cfg;
            cfg.name             = item.at("name").get<std::string>();
            std::cout << "{MODLE}}}} " << cfg.name << std::endl;
            cfg.modelPath        = item.at("path").get<std::string>();
            cfg.modifyAnnotation = item.value("modify_annotation", true);
            cfg.inputFormat      = item.at("inputFormat").get<std::vector<std::string>>();
            cfg.outputFormat     = item.at("outputFormat").get<std::vector<std::string>>();

            // scaleInput: flatten [min1,max1,min2,max2,...]
            for (size_t i = 0; i + 1 < item.at("scaleInput").size(); i += 2) {
                cfg.scaleInput.emplace_back(
                    item["scaleInput"][i].get<float>(),
                    item["scaleInput"][i+1].get<float>()
                );
            }
            // scaleOutput
            for (size_t i = 0; i + 1 < item.at("scaleOutput").size(); i += 2) {
                cfg.scaleOutput.emplace_back(
                    item["scaleOutput"][i].get<float>(),
                    item["scaleOutput"][i+1].get<float>()
                );
            }
            if (cfg.modifyAnnotation)
                registerModel(cfg);
        }
    }
}

void MlModel::registerModel(const ModelConfig& cfg) {
    models_[cfg.name] = {cfg, nullptr};
    if (!lazyLoad_) {
        // immediate load
        std::cout << "loading model for " << cfg.name << std::endl;
        models_[cfg.name].second = std::make_shared<fdeep::model>(
            fdeep::load_model(cfg.modelPath)
        );
    }
}

std::shared_ptr<fdeep::model> MlModel::getModel(const std::string& name) {
    auto it = models_.find(name);
    if (it == models_.end())
        throw std::runtime_error("Model not registered: " + name);
    auto &entry = it->second;
    if (!entry.second) {
        // lazy load on first use
        entry.second = std::make_shared<fdeep::model>(
            fdeep::load_model(entry.first.modelPath)
        );
    }
    return entry.second;
}

bool MlModel::modelAvailable(const std::string& modelName) const {
    return models_.find(modelName) != models_.end();
}


float MlModel::minMaxScale(float x, float minVal, float maxVal) const {
    return (x - minVal) / (maxVal - minVal);
}

float MlModel::inverseMinMaxScale(float s, float minVal, float maxVal) const {
    return s * (maxVal - minVal) + minVal;
}

std::pair<float, float> MlModel::calculateSkew(const std::vector<float*>& annos) const {
    if (!annos[0] || !annos[1]) return {0.0f, 0.0f};
    return { annos[0][0] - annos[1][0], annos[0][1] - annos[1][1] };
}

std::vector<float> MlModel::predict(const std::string &modelName,
                                    const std::vector<float> &input_data) {
    auto mdl = getModel(modelName);
    // wrap in a single tensor of shape [input_data.size()]
    fdeep::tensor t(fdeep::tensor_shape(input_data.size()), input_data);
    const auto res = mdl->predict({ t });
    // res is a vector<fdeep::tensor>, but frugally-deep packs single-output
    std::vector<float> out;
    for (auto &ot : res) {
        auto v = ot.to_vector();
        out.insert(out.end(), v.begin(), v.end());
    }
    if (out.empty())
        throw std::runtime_error("Empty prediction from model " + modelName);
    return out;
}

std::vector<float> MlModel::constructInput(const ModelConfig& cfg,
                                           float load,
                                           const std::vector<float*>& annos,
                                           const std::vector<float*>& slews) {
    std::vector<float> in;
    for (size_t i = 0; i < cfg.inputFormat.size(); ++i) {
        auto key = cfg.inputFormat[i];
        auto [mn, mx] = cfg.scaleInput[i];
        if (key == "load")
            in.push_back(minMaxScale(load, mn, mx));
        else if (key == "slew_a")
            in.push_back(minMaxScale(slews[0][1], mn, mx));
        else if (key == "slew_b")
            in.push_back(minMaxScale(slews[1][1], mn, mx));
        else if (key == "skew_ab")
            in.push_back(minMaxScale(calculateSkew(annos).first, mn, mx));
        else
            std::cerr << "[Warn] Unknown key " << key << std::endl;
    }
    return in;
}

void MlModel::decodeOutput(const ModelConfig& cfg,
                           const std::vector<float>& pred,
                           float* anno, float* slew) {
    for (size_t i = 0; i < cfg.outputFormat.size(); ++i) {
        auto key = cfg.outputFormat[i];
        auto [mn, mx] = cfg.scaleOutput[i];
        float v = inverseMinMaxScale(pred[i], mn, mx);
        if (key == "rise_delay") anno[0] = v;
        else if (key == "rise_slew") slew[0] = v;
    }
}

void MlModel::Modify(DataToModel* data) {
    auto it = models_.find(data->gate_name);
    if (it == models_.end()) {
        std::cerr << "[Error] no model for " << data->gate_name << std::endl;
        return;
    }
    auto const& cfg = it->second.first;
    if (!cfg.modifyAnnotation) {
        // JSON said no modify
        return;
    }

    // Gather A/B data
    std::vector<float*> Aann = { data->getOriginalArrivalA(), data->getOriginalArrivalB() };
    std::vector<float*> Bann = { data->getOriginalArrivalB(), data->getOriginalArrivalA() };
    std::vector<float*> Aslew = {
        new float[2]{ data->getSlewA().first, data->getSlewA().second },
        new float[2]{ data->getSlewB().first, data->getSlewB().second }
    };
    std::vector<float*> Bslew = {
        new float[2]{ data->getSlewB().first, data->getSlewB().second },
        new float[2]{ data->getSlewA().first, data->getSlewA().second }
    };

    // Predict for A
    auto pA = predict(cfg.name, constructInput(cfg, data->getLoadCap(), Aann, Aslew));
    // And for B
    auto pB = predict(cfg.name, constructInput(cfg, data->getLoadCap(), Bann, Bslew));

    // Allocate output arrays
    float* outA = new float[4]{-1,-1,-1,-1};
    float* outB = new float[4]{-1,-1,-1,-1};
    float* sA   = new float[2]{-1,-1};
    float* sB   = new float[2]{-1,-1};

    decodeOutput(cfg, pA, outA, sA);
    decodeOutput(cfg, pB, outB, sB);

    data->setModifiedArrivalA(outA);
    data->setModifiedSlewA(sA[0], sA[1]);
    data->setModifiedArrivalB(outB);
    data->setModifiedSlewB(sB[0], sB[1]);

    delete[] Aslew[0]; delete[] Aslew[1];
    delete[] Bslew[0]; delete[] Bslew[1];
}

