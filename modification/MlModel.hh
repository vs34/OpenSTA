#ifndef MLMODEL_HH
#define MLMODEL_HH

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <nlohmann/json.hpp>
#include <fdeep/fdeep.hpp>

struct ModelConfig {
    std::string name;
    std::string modelPath;
    bool modifyAnnotation = true;
    std::vector<std::string> inputFormat;
    std::vector<std::string> outputFormat;
    // Each pair is (min, max) for the corresponding input/​output
    std::vector<std::pair<float, float>> scaleInput;
    std::vector<std::pair<float, float>> scaleOutput;
};

class DataToModel;

class MlModel {
public:
    // Singleton accessor
    static MlModel* getInstance();

    // Load the JSON configuration (lazyLoad, model list, scales, flags)
    void loadConfigurations(const std::string& configFile);

    // Run raw fdeep prediction on a flat input vector
    std::vector<float> predict(const std::string& modelName,
                               const std::vector<float>& input_data);

    // Perform A/B-swap dual inference and update DataToModel
    void Modify(DataToModel* data);

    bool modelAvailable(const std::string& modelName) const;

    // Scaling helpers
    float minMaxScale(float x, float minVal, float maxVal) const;
    float inverseMinMaxScale(float s, float minVal, float maxVal) const;

    // Compute skew from two arrival arrays
    std::pair<float, float> calculateSkew(const std::vector<float*>& annotations) const;

private:
    MlModel();  // private ctor for singleton
    static MlModel* instance_;

    // Register a parsed ModelConfig (no immediate load)
    void registerModel(const ModelConfig& cfg);

    // Get (and, if needed, lazy‐load) the fdeep::model
    std::shared_ptr<fdeep::model> getModel(const std::string& name);

    // Build model inputs from raw data + scaling
    std::vector<float> constructInput(const ModelConfig& cfg,
                                      float load,
                                      const std::vector<float*>& annos,
                                      const std::vector<float*>& slews);

    // Decode flat prediction vector back to arrival/slew values
    void decodeOutput(const ModelConfig& cfg,
                      const std::vector<float>& pred,
                      float* anno,
                      float* slew);

    bool lazyLoad_ = true;
    std::map<std::string,
             std::pair<ModelConfig, std::shared_ptr<fdeep::model>>>
        models_;
};

#endif // MLMODEL_HH

