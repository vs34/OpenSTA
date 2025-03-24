#ifndef MLMODEL_H
#define MLMODEL_H

#include <fdeep/fdeep.hpp>
#include <vector>

class MlModel {
private:
    MlModel(); // Private constructor for Singleton pattern
    fdeep::model model_;

public:
    static MlModel& getInstance();
    float predict(float* annotation);
};

#endif // MLMODEL_H

