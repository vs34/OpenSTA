#ifndef MLMODEL_H
#define MLMODEL_H

class MlModel {
private:
    MlModel(); // Private constructor for Singleton pattern

public:
    static MlModel& getInstance();
    float predict(float* annotation);
};

#endif // MLMODEL_H

