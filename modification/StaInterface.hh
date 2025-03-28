#ifndef STAINTERFACE_H
#define STAINTERFACE_H

#include "Graph.hh"
#include "MlModel.hh"
class StaGraph;

class StaInterface {
private:
    sta::Graph* sta_graph_;
    MlModel ml_model_;    
    float* getAnnotationArray(sta::Vertex *vertex);
    void setAnnotationArray(sta::Vertex *vertex, float *updated_annotation);

public:
    StaInterface(sta::Graph* graph);
    void updateGraph();
};

#endif // STAINTERFACE_H

