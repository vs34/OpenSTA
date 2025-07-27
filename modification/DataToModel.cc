
// DataToModel.cc
#include "DataToModel.hh"
#include <algorithm> // for std::copy

DataToModel::DataToModel(sta::Vertex* zn)
    : Zn(zn),
      load_cap_A(-1.0f),
      load_cap_B(-1.0f),
      load_cap_Zn(-1.0f),
      A(nullptr),
      B(nullptr),
      original_arrival_A({-1.0f, -1.0f, -1.0f, -1.0f}),
      original_arrival_B({-1.0f, -1.0f, -1.0f, -1.0f}),
      original_arrival_Zn({-1.0f, -1.0f, -1.0f, -1.0f}),
      slew_A({-1.0f, -1.0f}),
      slew_B({-1.0f, -1.0f}),
      slew_Zn({-1.0f, -1.0f}),
      modified_arrival_A({-1.0f, -1.0f, -1.0f, -1.0f}),
      modified_arrival_B({-1.0f, -1.0f, -1.0f, -1.0f}),
      modified_arrival_Zn({-1.0f, -1.0f, -1.0f, -1.0f}),
      input_delay_A({-1.0f, -1.0f, -1.0f, -1.0f}),
      input_delay_B({-1.0f, -1.0f, -1.0f, -1.0f}),
      modified_slew_A({-1.0f, -1.0f}),
      modified_slew_B({-1.0f, -1.0f}),
      modified_slew_Zn({-1.0f, -1.0f})
{
}

void DataToModel::setGateName(const char* name) {
    gate_name = name;
}

void DataToModel::setLoadCapA(float cap) { load_cap_A = cap; }
void DataToModel::setLoadCapB(float cap) { load_cap_B = cap; }
void DataToModel::setLoadCapZn(float cap) { load_cap_Zn = cap; }
void DataToModel::setA(sta::Vertex* a) { A = a; }
void DataToModel::setB(sta::Vertex* b) { B = b; }

void DataToModel::setOriginalArrivalA(const float* arrival) {
    std::copy(arrival, arrival + 4, original_arrival_A.begin());
}
void DataToModel::setOriginalArrivalB(const float* arrival) {
    std::copy(arrival, arrival + 4, original_arrival_B.begin());
}
void DataToModel::setOriginalArrivalZn(const float* arrival) {
    std::copy(arrival, arrival + 4, original_arrival_Zn.begin());
}

void DataToModel::setInputDelayA(const float* arrival) {
    std::copy(arrival, arrival + 4, input_delay_A.begin());
}

void DataToModel::setInputDelayB(const float* arrival) {
    std::copy(arrival, arrival + 4, input_delay_B.begin());
}

void DataToModel::setSlewA(float rise, float fall) { slew_A = {rise, fall}; }
void DataToModel::setSlewB(float rise, float fall) { slew_B = {rise, fall}; }
void DataToModel::setSlewZn(float rise, float fall) { slew_Zn = {rise, fall}; }

void DataToModel::setModifiedArrivalA(const float* arrival) {
    std::copy(arrival, arrival + 4, modified_arrival_A.begin());
}
void DataToModel::setModifiedArrivalB(const float* arrival) {
    std::copy(arrival, arrival + 4, modified_arrival_B.begin());
}
void DataToModel::setModifiedArrivalZn(const float* arrival) {
    std::copy(arrival, arrival + 4, modified_arrival_Zn.begin());
}

void DataToModel::setModifiedSlewA(float rise, float fall) { modified_slew_A = {rise, fall}; }
void DataToModel::setModifiedSlewB(float rise, float fall) { modified_slew_B = {rise, fall}; }
void DataToModel::setModifiedSlewZn(float rise, float fall) { modified_slew_Zn = {rise, fall}; }

// Getters
sta::Vertex* DataToModel::getZn() const { return Zn; }
float DataToModel::getLoadCapA() const { return load_cap_A; }
float DataToModel::getLoadCapB() const { return load_cap_B; }
float DataToModel::getLoadCapZn() const { return load_cap_Zn; }
sta::Vertex* DataToModel::getA() const { return A; }
sta::Vertex* DataToModel::getB() const { return B; }

float* DataToModel::getOriginalArrivalA() { return original_arrival_A.data(); }
float* DataToModel::getOriginalArrivalB() { return original_arrival_B.data(); }
float* DataToModel::getOriginalArrivalZn() { return original_arrival_Zn.data(); }

float* DataToModel::getInputDelayA() { return input_delay_A.data(); }
float* DataToModel::getInputDelayB() { return input_delay_B.data(); }
std::pair<float, float> DataToModel::getSlewA() const { return slew_A; }
std::pair<float, float> DataToModel::getSlewB() const { return slew_B; }
std::pair<float, float> DataToModel::getSlewZn() const { return slew_Zn; }

float* DataToModel::getModifiedArrivalA() { return modified_arrival_A.data(); }
float* DataToModel::getModifiedArrivalB() { return modified_arrival_B.data(); }
float* DataToModel::getModifiedArrivalZn() { return modified_arrival_Zn.data(); }

std::pair<float, float> DataToModel::getModifiedSlewA() const { return modified_slew_A; }
std::pair<float, float> DataToModel::getModifiedSlewB() const { return modified_slew_B; }
std::pair<float, float> DataToModel::getModifiedSlewZn() const { return modified_slew_Zn; }

