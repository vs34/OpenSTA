#ifndef MLMODEL_H
#define MLMODEL_H

#include <fdeep/fdeep.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <tuple>

// We'll use nlohmann/json for config parsing
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// Structure to hold configuration for each model
struct ModelConfig {
    std::string name;         // Model name
    std::string modelPath;    // Path to the model JSON file (fdeep format)
    std::vector<std::string> inputFormat;  // List of input parameter names (for documentation)
    std::vector<std::string> outputFormat; // List of output names
};

class MlModel {
private:
    // Private constructor for singleton
    MlModel();
    
    // Global flag to control lazy loading
    bool lazyLoad_;

    // Map from model name to a pair of configuration and loaded model pointer.
    // Models are loaded on demand if lazyLoad_ is true.
    std::map<std::string, std::pair<ModelConfig, std::shared_ptr<fdeep::model>>> models_;

    // Loads a model given its configuration and returns a shared pointer.
    std::shared_ptr<fdeep::model> load_model(const ModelConfig &config);

    // Loads the configuration file and registers the models.
    void loadConfigurations(const std::string &configFile);
    std::pair<float,float> calculateSkew(std::vector<float*> annotation);

public:
    // Get singleton instance
    static MlModel& getInstance();

    // Registers a new model with its configuration.
    void registerModel(const ModelConfig &config);

    // Returns the model pointer for the given name, loading it if necessary.
    std::shared_ptr<fdeep::model> getModel(const std::string &name);

    // Predict using the specified model and input data.
    // 'input_data' is a vector of floats prepared for the model.
    float predict(const std::string &modelName, const std::vector<float> &input_data);

    // Returns a tuple with some status flags and predictions.
    // For demonstration, we return a tuple of three booleans and three float pointers.
    std::tuple<bool, bool, bool, float, float, float> 
    getModelAnnotation(const std::string &modelToUse,
                       const std::vector<float*>& annotations,
                       const std::vector<float>& load_cap,
                       const std::vector<float*>& slew);
};

#endif // MLMODEL_H

