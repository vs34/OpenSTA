#ifndef STAINTERFACE_H
#define STAINTERFACE_H

#include "Graph.hh"
#include "Network.hh"
#include "MlModel.hh"
#include "sta/Liberty.hh"
#include "sta/PortDirection.hh"

class StaGraph;

class StaInterface {
private:
    sta::Graph* sta_graph_;
    sta::Network* net_netlist_;
    MlModel ml_model_;    
    float* getAnnotationArray(sta::Vertex *vertex);
    float* getSlew(sta::Vertex *vertex);
    float getLoadCapacitance(sta::Pin *pin);
    void updateAnnotation_fanout_from_fanin(sta::Vertex *vertex);
    void updateAnnotation_fanin_from_fanin(sta::Vertex *fanout);
    void setSlew(sta::Vertex *vertex,float *newslew);
    void recursiveUpdate(sta::Vertex *vertex);
    void debugCout(sta::Vertex *vertex);
    void pinDirectionCout(sta::Pin *pin);
    void setAnnotationArray(sta::Vertex *vertex, float *updated_annotation);

public:
    StaInterface(sta::Graph* graph, sta::Network* network);
    void updateGraph();
    void hackModelUpdate(sta::Vertex *vertex);
    void setGraph(sta::Graph* graph);
    std::map<sta::Vertex*,bool> visitedDP;
    std::map<sta::Vertex*,bool> cycleDect;
};

#endif // STAINTERFACE_H

