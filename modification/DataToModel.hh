#ifndef DATATOMODEL_H
#define DATATOMODEL_H

#include <utility>  // for std::pair
#include "sta/Liberty.hh"
#include "Graph.hh"
#include "Network.hh"

class DataToModel {
public:
    // Constructor
    explicit DataToModel(sta::Vertex* zn);

    const char* gate_name;
    // Data members
    sta::Vertex* Zn;
    float load_cap;
    sta::Vertex* A;
    sta::Vertex* B;
    float* original_arrival_A;
    float* original_arrival_B;
    std::pair<float, float> slew_A;
    std::pair<float, float> slew_B;
    float* modified_arrival_A;
    float* modified_arrival_B;
    std::pair<float, float> modified_slew_A;
    std::pair<float, float> modified_slew_B;

    // Setters
    void setGateName(const char* name);
    void setLoadCap(float cap);
    void setA(sta::Vertex* a);
    void setB(sta::Vertex* b);
    void setOriginalArrivalA(float* arrival);
    void setOriginalArrivalB(float* arrival);
    void setSlewA(float rise, float fall);
    void setSlewB(float rise, float fall);
    void setModifiedArrivalA(float* arrival);
    void setModifiedArrivalB(float* arrival);
    void setModifiedSlewA(float rise, float fall);
    void setModifiedSlewB(float rise, float fall);

    // Getters
    sta::Vertex* getZn() const;
    float getLoadCap() const;
    sta::Vertex* getA() const;
    sta::Vertex* getB() const;
    float* getOriginalArrivalA() const;
    float* getOriginalArrivalB() const;
    std::pair<float, float> getSlewA() const;
    std::pair<float, float> getSlewB() const;
    float* getModifiedArrivalA() const;
    float* getModifiedArrivalB() const;
    std::pair<float, float> getModifiedSlewA() const;
    std::pair<float, float> getModifiedSlewB() const;
};

#endif // DATATOMODEL_H

