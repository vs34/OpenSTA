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
        return sta_graph_->arrivals(vertex);
}


void StaInterface::checkArrivalArray(sta::Vertex *vertex) {

    float* arrivals = sta_graph_->arrivals(vertex);
    std::cout << "[annotations currently] arrivals = "<< arrivals[0] << " " << arrivals[1] << " " << arrivals[2] << " " << arrivals[3] << std::endl;

    return;
}

// Return the slew pointer for a vertex.
float* StaInterface::getSlew(sta::Vertex *vertex) {
    if (vertex == nullptr)
        return nullptr;
    return vertex->slews();
}

void StaInterface::setSlew(sta::Vertex *vertex, std::pair<float, float> newslew) {
    float* orig = getSlew(vertex);
    float* ns = (float*)malloc(2*sizeof(float));
    if (newslew.first != -1)
        ns[0] = newslew.first;
    else
        ns[0] = orig[0];
    if (newslew.second != -1)
        ns[1] = newslew.second;
    else
        ns[1] = orig[1];

    // if (newslew.first != -1)
        // vertex->slews()[0] = newslew.first;

    // if (newslew.second != -1)
        // vertex->slews()[1] = newslew.second;
    sta_graph_->changeSlews(vertex,ns);
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
void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation, float *old_annotation) {
    float* arrivals = sta_graph_->arrivals(vertex);
    float new_arrivals[4];
    if (arrivals != nullptr) {
        bool anotation_update = false;
        for (int a = 0 ; a < 4 ; a++){
            if (new_annotation[a] != -1){  // this is set my model class if -1 no change
                // std::cout << "[[[[[[[[UPDATE]]]]]]]] annotation change "<<arrivals[a]<<" to "<<new_annotation[a]<<std::endl;
                new_arrivals[a] = new_annotation[a];
                anotation_update = true;
            }
            else
                new_arrivals[a] = arrivals[a];
        }
    // vertex->setArrivals(new_arrivals);
    sta_graph_->changeArrivals(vertex,new_arrivals);

    if (anotation_update)
        std::cout << "[UPDATE] annotaion updated for "<< net_netlist_->pathName(vertex->pin()) <<  std::endl;
        
    } else {
        std::cerr << "Error: arrivals() returned nullptr!" << std::endl;
    }

}

const char* StaInterface::getGateName(sta::Vertex *vertex){
    if (vertex == nullptr)
        return nullptr;
    sta::Instance *instance = net_netlist_->instance(vertex->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    return net_netlist_->name(cell);
}

bool StaInterface::updateAnnotation_fanin_from_fanin(DataToModel* data) {
    // data->setLoadCap(getLoadCapacitance(data->getZn())); // my dumb assumtion that the load cap is independent of pin
    // std::cout << "the cap Zn" << getLoadCapacitance(data->getZn()) << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    data->setGateName(getGateName(data->getZn()));

    sta::VertexInEdgeIterator edge_iter(data->getZn(), sta_graph_);
    while (edge_iter.hasNext()) {
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        if (data->getB() == nullptr)
            data->setB(prev_vertex);
        else
            data->setA(prev_vertex);
    }
    data->setLoadCapA(getLoadCapacitance(data->getA())); 
    data->setLoadCapB(getLoadCapacitance(data->getB()));
    auto it = visited_vertex_.find(data->getA());
    if (it != visited_vertex_.end())
        return false;

    data->setOriginalArrivalA(getAnnotationArray(data->getA()));
    data->setOriginalArrivalB(getAnnotationArray(data->getB()));
    float* sl = getSlew(data->getA());
    if (sl)
        data->setSlewA(sl[0],sl[1]);
    sl = getSlew(data->getB());
    if (sl)
        data->setSlewB(sl[0],sl[1]);

    ml_model_->Modify(data);
    // std::cout << "[setAnnotaiton] setting annotation for A"<< std::endl << "old anno";
    // checkArrivalArray(data->getA());
    setAnnotationArray(data->getA(), data->getModifiedArrivalA(),data->getOriginalArrivalA());
    // std::cout << "new anno";
    // checkArrivalArray(data->getA());
    // std::cout << "[setAnnotaiton] setting annotation for B"<< std::endl << "old anno";
    // checkArrivalArray(data->getB()); 
    setAnnotationArray(data->getB(), data->getModifiedArrivalB(), data->getOriginalArrivalB());
    // std::cout << "new anno";
    checkArrivalArray(data->getB());
    setSlew(data->getA(), data->getModifiedSlewA());
    setSlew(data->getB(), data->getModifiedSlewB());
    return true;
}

void StaInterface::hackModelUpdate(sta::Vertex *vertex) {
    const sta::PortDirection *direction = net_netlist_->direction(vertex->pin());
    if (direction->isOutput()) {
        if (vertex->hasFanout()) {
            if (ml_model_->modelAvailable(getGateName(vertex))){
                DataToModel* data = new DataToModel(vertex);
                bool updated = updateAnnotation_fanin_from_fanin(data);
                // delete data;
                if (updated){
                    visited_vertex_[data->getA()] = data;
                    visited_vertex_[data->getB()] = data;
                }
            }
        }
    }
}


void StaInterface::updateReInitialized(sta::Vertex *vertex) {
    // Look up whether this fan-in was part of an output we ML-touched
    auto it = visited_vertex_.find(vertex);
    if (it == visited_vertex_.end())
        return;
    DataToModel* data = it->second;

    // If this is pin A, apply A’s modified vs. original; else B’s
    if (vertex == data->getA()) {
        setAnnotationArray(data->getA(),
                           data->getModifiedArrivalA(),
                           data->getOriginalArrivalA());
        setSlew(data->getA(), data->getModifiedSlewA());
    } else if (vertex == data->getB()) {
        setAnnotationArray(data->getB(),
                           data->getModifiedArrivalB(),
                           data->getOriginalArrivalB());
        setSlew(data->getB(), data->getModifiedSlewB());
    }
}
