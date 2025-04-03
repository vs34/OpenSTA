#ifndef STAINTERFACE_H
#define STAINTERFACE_H

#include "Graph.hh"
#include "Network.hh"
#include "MlModel.hh"
#include "sta/Liberty.hh"

class StaGraph;

class StaInterface {
private:
    sta::Graph* sta_graph_;
    sta::Network* net_netlist_;
    MlModel ml_model_;    
    float* getAnnotationArray(sta::Vertex *vertex);
    float* getSlew(sta::Vertex *vertex);
    float getLoadCapacitance(sta::Pin *pin);
    void updateAnnotation(sta::Vertex *vertex);
    void recursiveUpdate(sta::Vertex *vertex);
    void debugCout(sta::Vertex *vertex);
    void setAnnotationArray(sta::Vertex *vertex, float *updated_annotation);

public:
    StaInterface(sta::Graph* graph, sta::Network* network);
    void updateGraph();
    std::map<sta::Vertex*,bool> visitedDP;
};

#endif // STAINTERFACE_H

