// DataToModel.hh
#ifndef DATATOMODEL_H
#define DATATOMODEL_H

#include <array>
#include <utility>  // for std::pair
#include "sta/Liberty.hh"
#include "Graph.hh"
#include "Network.hh"

class DataToModel {
public:
    // Constructor
    explicit DataToModel(sta::Vertex* zn);

    // Data members
    const char* gate_name = nullptr;
    sta::Vertex* Zn;           // ideally this should be a pin pointer
    float load_cap_A;
    float load_cap_B;
    float load_cap_Zn;
    sta::Vertex* A;
    sta::Vertex* B;

    // Store raw arrival data internally (4 elements per annotation)
    std::array<float, 4> original_arrival_A;
    std::array<float, 4> original_arrival_B;
    std::array<float, 4> original_arrival_Zn;
    std::pair<float, float> slew_A;
    std::pair<float, float> slew_B;
    std::pair<float, float> slew_Zn;

    std::array<float, 4> modified_arrival_A;
    std::array<float, 4> modified_arrival_B;
    std::array<float, 4> modified_arrival_Zn;
    std::pair<float, float> modified_slew_A;
    std::pair<float, float> modified_slew_B;
    std::pair<float, float> modified_slew_Zn;

    // Setters
    void setGateName(const char* name);
    void setLoadCapA(float cap);
    void setLoadCapB(float cap);
    void setLoadCapZn(float cap);
    void setA(sta::Vertex* a);
    void setB(sta::Vertex* b);
    void setOriginalArrivalA(const float* arrival);
    void setOriginalArrivalB(const float* arrival);
    void setOriginalArrivalZn(const float* arrival);
    void setSlewA(float rise, float fall);
    void setSlewB(float rise, float fall);
    void setSlewZn(float rise, float fall);
    void setModifiedArrivalA(const float* arrival);
    void setModifiedArrivalB(const float* arrival);
    void setModifiedArrivalZn(const float* arrival);
    void setModifiedSlewA(float rise, float fall);
    void setModifiedSlewB(float rise, float fall);
    void setModifiedSlewZn(float rise, float fall);

    // Getters
    sta::Vertex* getZn() const;
    float getLoadCapA() const;
    float getLoadCapB() const;
    float getLoadCapZn() const;
    sta::Vertex* getA() const;
    sta::Vertex* getB() const;
    // Return non-const pointers to support legacy APIs
    float* getOriginalArrivalA();
    float* getOriginalArrivalB();
    float* getOriginalArrivalZn();
    std::pair<float, float> getSlewA() const;
    std::pair<float, float> getSlewB() const;
    std::pair<float, float> getSlewZn() const;
    float* getModifiedArrivalA();
    float* getModifiedArrivalB();
    float* getModifiedArrivalZn();
    std::pair<float, float> getModifiedSlewA() const;
    std::pair<float, float> getModifiedSlewB() const;
    std::pair<float, float> getModifiedSlewZn() const;
};

#endif // DATATOMODEL_H

