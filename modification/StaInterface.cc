#include "StaInterface.hh"
#include "MlModel.hh"
#include <iostream>
#include <stdio.h>

// Constructor: initialize the STA graph and network pointer, and get the singleton MlModel instance.
StaInterface::StaInterface(sta::Graph* graph, sta::Network* network) :
    sta_graph_(graph),
    net_netlist_(network),
    ml_model_(MlModel::getInstance())
{}

void StaInterface::setGraph(sta::Graph *newgraph){
    this->sta_graph_ = newgraph;
}

// Return the arrivals array for a vertex.
float* StaInterface::getAnnotationArray(sta::Vertex *vertex) {
    if (vertex == nullptr)
        return nullptr;
    float* arrivals = new float[4] ; // min/max rise/fall
    sta::Path* path_array = sta_graph_->paths(vertex);

    if (!path_array) return nullptr;

    // The number of paths is architecture-specific.
    // Commonly it's 4 (rise/fall × min/max).
    for (int i = 0; i < 4; ++i) {
        arrivals[i] = path_array[i].arrival();
    }
    std::cout << "[annotation array debug] arrivals = "<< arrivals[0] << " " << arrivals[1] << " " << arrivals[2] << " " << arrivals[3] << std::endl;

    return arrivals;
}

// Return the slew pointer for a vertex.
float* StaInterface::getSlew(sta::Vertex *vertex) {
    if (vertex == nullptr)
        return nullptr;
    return vertex->slews();
}

void StaInterface::setSlew(sta::Vertex *vertex, std::pair<float, float> newslew) {
    if (newslew.first != -1)
        vertex->slews()[0] = newslew.first;

    if (newslew.second != -1)
        vertex->slews()[1] = newslew.second;
}

// Get the load capacitance for a pin by querying the LibertyPort.
float StaInterface::getLoadCapacitance(sta::Vertex *vertex) {
    if (vertex == nullptr)
        return -1.0f;
    sta::Pin* pin = vertex->pin();
    sta::LibertyPort *lib_port = net_netlist_->libertyPort(pin);
    return lib_port->capacitance();
}

// Sets the annotation array for a vertex.
void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation) {
    if (new_annotation == nullptr){
        std::cout << "[setAnnotaiton] the new_annotation is nullptr" << std::endl;
        return;
    }

    // Assume arrivals are obtained here
    sta::Path* path_array = sta_graph_->paths(vertex);


    // The number of paths is architecture-specific.
    // Commonly it's 4 (rise/fall × min/max).
    for (int i = 0; i < 4; ++i) {
        if (new_annotation[i] != -1)
            std::cout << "[setAnnotaiton] annotations changes from " << path_array[i].arrival() << " to " << new_annotation[i] << std::endl;
            path_array[i].setArrival(new_annotation[i]);
    }
}

const char* StaInterface::getGateName(sta::Vertex *vertex){
    if (vertex == nullptr)
        return nullptr;
    sta::Instance *instance = net_netlist_->instance(vertex->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    return net_netlist_->name(cell);
}

void StaInterface::updateAnnotation_fanin_from_fanin(DataToModel* data) {
    data->setLoadCap(getLoadCapacitance(data->getZn()));
    data->setGateName(getGateName(data->getZn()));

    sta::VertexInEdgeIterator edge_iter(data->getZn(), sta_graph_);
    while (edge_iter.hasNext()) {
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        if (data->getA() == nullptr)
            data->setA(prev_vertex);
        else
            data->setB(prev_vertex);
    }

    data->setOriginalArrivalA(getAnnotationArray(data->getA()));
    data->setOriginalArrivalB(getAnnotationArray(data->getB()));
    float* sl = getSlew(data->getA());
    if (sl)
        data->setSlewA(sl[0],sl[1]);
    sl = getSlew(data->getB());
    if (sl)
        data->setSlewB(sl[0],sl[1]);

    ml_model_->Modify(data);
    std::cout << "[setAnnotaiton] setting annotation for A"<< std::endl;
    setAnnotationArray(data->getA(), data->getModifiedArrivalA());
    std::cout << "[setAnnotaiton] setting annotation for B"<< std::endl;
    setAnnotationArray(data->getB(), data->getModifiedArrivalB());
    setSlew(data->getA(), data->getModifiedSlewA());
    setSlew(data->getB(), data->getModifiedSlewB());
}

void StaInterface::hackModelUpdate(sta::Vertex *vertex) {
    const sta::PortDirection *direction = net_netlist_->direction(vertex->pin());
    if (direction->isOutput()) {
        if (vertex->hasFanout()) {
            if (ml_model_->modelAvailable(getGateName(vertex))){
                DataToModel* data = new DataToModel(vertex);
                updateAnnotation_fanin_from_fanin(data);
                delete data;
            }
        }
    }
}

