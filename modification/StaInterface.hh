#ifndef STAINTERFACE_H
#define STAINTERFACE_H

#include "Graph.hh"
#include "Network.hh"
#include "MlModel.hh"
#include "DataToModel.hh"
#include "sta/Liberty.hh"
#include "sta/PortDirection.hh"

class StaInterface {
private:
    sta::Graph* sta_graph_;
    sta::Network* net_netlist_;
    MlModel* ml_model_;    
    std::unordered_map<sta::Vertex*, DataToModel*> visited_vertex_;


    float* getAnnotationArray(sta::Vertex *vertex);
    float* getSlew(sta::Vertex *vertex);
    void setSlew(sta::Vertex *vertex, std::pair<float, float> newslew);
    void setAnnotationArray(sta::Vertex *vertex, float *new_annotation, float *old_annotation);
    const char* getGateName(sta::Vertex *vertex);
    bool updateAnnotation_fanin_from_fanin(DataToModel* data);
    float getLoadCapacitance(sta::Vertex *vertex);
    void checkArrivalArray(sta::Vertex *vertex);

public:
    StaInterface(sta::Graph* graph, sta::Network* network);
    void setGraph(sta::Graph* graph);
    // void updateGraph();
    void hackModelUpdate(sta::Vertex *vertex);
    void updateReInitialized(sta::Vertex *vertex);
    // std::map<sta::Vertex*, bool> visitedDP;
    // std::map<sta::Vertex*, bool> cycleDect;
};

#endif // STAINTERFACE_H

