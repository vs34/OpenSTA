#include "DataToModel.hh"

// Constructor
DataToModel::DataToModel(sta::Vertex* zn)
    : Zn(zn),
      load_cap(-1.0f),
      A(nullptr), B(nullptr),
      original_arrival_A(nullptr),
      original_arrival_B(nullptr),
      slew_A{-1.0f, -1.0f},
      slew_B{-1.0f, -1.0f},
      modified_arrival_A(nullptr),
      modified_arrival_B(nullptr),
      modified_slew_A{-1.0f, -1.0f},
      modified_slew_B{-1.0f, -1.0f},
      gate_name(nullptr)
{}

// Setters
void DataToModel::setGateName(const char* name) {
    gate_name = name;
}

void DataToModel::setLoadCap(float cap) {
    load_cap = cap;
}

void DataToModel::setA(sta::Vertex* a) {
    A = a;
}

void DataToModel::setB(sta::Vertex* b) {
    B = b;
}

void DataToModel::setOriginalArrivalA(float* arrival) {
    original_arrival_A = arrival;
}

void DataToModel::setOriginalArrivalB(float* arrival) {
    original_arrival_B = arrival;
}

void DataToModel::setSlewA(float rise, float fall) {
    slew_A = {rise, fall};
}

void DataToModel::setSlewB(float rise, float fall) {
    slew_B = {rise, fall};
}

void DataToModel::setModifiedArrivalA(float* arrival) {
    modified_arrival_A = arrival;
}

void DataToModel::setModifiedArrivalB(float* arrival) {
    modified_arrival_B = arrival;
}

void DataToModel::setModifiedSlewA(float rise, float fall) {
    modified_slew_A = {rise, fall};
}

void DataToModel::setModifiedSlewB(float rise, float fall) {
    modified_slew_B = {rise, fall};
}

// Getters
sta::Vertex* DataToModel::getZn() const {
    return Zn;
}

float DataToModel::getLoadCap() const {
    return load_cap;
}

sta::Vertex* DataToModel::getA() const {
    return A;
}

sta::Vertex* DataToModel::getB() const {
    return B;
}

float* DataToModel::getOriginalArrivalA() const {
    return original_arrival_A;
}

float* DataToModel::getOriginalArrivalB() const {
    return original_arrival_B;
}

std::pair<float, float> DataToModel::getSlewA() const {
    return slew_A;
}

std::pair<float, float> DataToModel::getSlewB() const {
    return slew_B;
}

float* DataToModel::getModifiedArrivalA() const {
    return modified_arrival_A;
}

float* DataToModel::getModifiedArrivalB() const {
    return modified_arrival_B;
}

std::pair<float, float> DataToModel::getModifiedSlewA() const {
    return modified_slew_A;
}

std::pair<float, float> DataToModel::getModifiedSlewB() const {
    return modified_slew_B;
}

