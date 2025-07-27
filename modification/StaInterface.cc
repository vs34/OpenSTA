#include "StaInterface.hh"
#include "MlModel.hh"
#include <iostream>
#include <stdio.h>

// Constructor: initialize the STA graph and network pointer, and get the singleton MlModel instance.
StaInterface::StaInterface(sta::Graph* graph, sta::Network* network,const sta::StaState* sta) :
    god_class_(sta),
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
     if (vertex == nullptr) // should not happen in real 
         return -1.0f;
     sta::Pin* pin = vertex->pin();
     sta::LibertyPort *lib_port = net_netlist_->libertyPort(pin);
     if (lib_port == nullptr) // checking
         return -1.0f;
     return lib_port->capacitance();
 }

// This is the corrected implementation for your class method.
// It assumes your StaInterface class has the member 'const sta::StaState* god_class_;'


// Sets the annotation array for a vertex.
void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation, float *old_annotation) {
    // float* arrivals = sta_graph_->arrivals(vertex);
    float* arrivals = old_annotation;
    float new_arrivals[4];
    if (arrivals != nullptr) {
        bool anotation_update = false;
        for (int a = 0 ; a < 4 ; a++){
            if (new_annotation[a] != -1){  // this is set my model class if -1 no change
                // std::cout << "[[[[[[[[UPDATE]]]]]]]] annotation change "<<arrivals[a]<<" to "<<new_annotation[a]<<std::endl;
                std::cout << "index is " << a << " value is " << new_annotation[a] << '\n';
                new_arrivals[a] = new_annotation[a];
                anotation_update = true;
                // arrivals[a] = arrivals[a];
            }
            else{
                new_arrivals[a] = arrivals[a];
            }
        }
    // vertex->setArrivals(new_arrivals);
    sta_graph_->changeArrivals(vertex,new_arrivals);
    std::cout << "old annotation " << old_annotation[0] << ' ' << old_annotation[1] << ' ' << old_annotation[2] << ' ' << old_annotation[3] << '\n';
    std::cout << "new annotation " << new_arrivals[0] << ' ' << new_arrivals[1] << ' ' << new_arrivals[2] << ' ' << new_arrivals[3] << '\n';
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
    // this function assume that the model modify the fan in annotations so that the 
    // new anotation calculated by the normal SIS flow of OpenSTA will accure MIS effects

    // data->setLoadCap(getLoadCapacitance(data->getZn())); // my dumb assumtion that the load cap is independent of pin
    data->setGateName(getGateName(data->getZn()));
    float cap_zn = getLoadCapacitance(data->getZn());
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
    std::cout << "the cap A  : " << getLoadCapacitance(data->getA()) << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    std::cout << "the cap B  : " << getLoadCapacitance(data->getB()) << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    std::cout << "the cap Zn : " << cap_zn << '\n'; // the Zn cap is 0 i thought it would be same for A,B
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

    ml_model_->Modify_in(data);
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



void StaInterface::debugCout(sta::Vertex *vertex) {
    std::cout << " ########### DEBUG ###########" << '\n';
    std::cout << "this pin " << net_netlist_->pathName(vertex->pin()) << std::endl;
    //std::cout << "Load Cap " << getLoadCapacitance(vertex->pin()) << std::endl;
    sta::Instance *instance = net_netlist_->instance(vertex->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    std::cout << "gate name " << net_netlist_->name(cell) << std::endl;
    std::cout << "load cap of the " << getLoadCapacitance(vertex) << '\n';
    std::cout << " ########### DEBUG ###########" << '\n';
}


bool StaInterface::updateAnnotation_out_pin(DataToModel *data){
    // this function assume the ML model is normal SPICE model 
    // so this get the annotations from the model givng fan in 

    // sta::VertexOutEdgeIterator out_iter(data->getZn(), sta_graph_);
    // while (out_iter.hasNext()) {
        // sta::Edge *out_edge = out_iter.next();
        // sta::Vertex *next_vertex = out_edge->from(sta_graph_); // the load cap should be here
        // debugCout(next_vertex);
    // }
    
    // data->setLoadCap(getLoadCapacitance(data->getZn())); // my dumb assumtion that the load cap is independent of pin
    data->setGateName(getGateName(data->getZn()));
    data->setLoadCapZn(getLoadCapacitance(data->getZn()));
    sta::VertexInEdgeIterator edge_iter(data->getZn(), sta_graph_);
    while (edge_iter.hasNext()) {
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        if (data->getB() == nullptr)
            data->setB(prev_vertex);
        else
            data->setA(prev_vertex);
    }
    data->setLoadCapA(getLoadCapacitance(data->getA())); // no need for this 
    data->setLoadCapB(getLoadCapacitance(data->getB())); // remove while opti.

    std:: cout << "cheking the annotaiton thiking that we could get input delay applyed by .tcl there\n";


    std::cout << "cheking for pin A " ;
    sta::VertexInEdgeIterator edgeA_iter(data->getA(), sta_graph_);
    while (edgeA_iter.hasNext()) {
        sta::Edge *nextA_edge = edgeA_iter.next();
        sta::Vertex *prev_vertex = nextA_edge->from(sta_graph_);
        float* temp = getAnnotationArray(prev_vertex);
        data->setInputDelayA(temp);
        for (int a = 0 ; a < 4 ; a++)
                std::cout << temp[a] << ' ';
        std::cout << '\n';
    }

    std::cout << "cheking for pin B " ;
    sta::VertexInEdgeIterator edgeB_iter(data->getB(), sta_graph_);
    while (edgeB_iter.hasNext()) {
        sta::Edge *nextB_edge = edgeB_iter.next();
        sta::Vertex *prev_vertex = nextB_edge->from(sta_graph_);
        float* temp = getAnnotationArray(prev_vertex);
        data->setInputDelayB(temp);
        for (int a = 0 ; a < 4 ; a++)
                std::cout << temp[a] << ' ';
        std::cout << '\n';
    }

    data->setOriginalArrivalA(getAnnotationArray(data->getA()));
    data->setOriginalArrivalB(getAnnotationArray(data->getB()));
    data->setOriginalArrivalZn(getAnnotationArray(data->getZn()));

    float* sl = getSlew(data->getA());
    if (sl)
        data->setSlewA(sl[0],sl[1]);
    sl = getSlew(data->getB());
    if (sl)
        data->setSlewB(sl[0],sl[1]);

    ml_model_->Modify_out(data);

    // std::cout << "the cap A  : " << data -> getOriginalArrivalA()[0] << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    // std::cout << "the cap B  : " << data -> getOriginalArrivalB()[0] << '\n';
    // std::cout << "the cap Zn : " << data -> getOriginalArrivalZn()[0] << '\n';

    // std::cout << "the cap A  : " << data -> getOriginalArrivalA()[1] << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    // std::cout << "the cap B  : " << data -> getOriginalArrivalB()[1] << '\n';
    // std::cout << "the cap Zn : " << data -> getOriginalArrivalZn()[1] << '\n';

    // std::cout << "the cap A  : " << data -> getOriginalArrivalA()[2] << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    // std::cout << "the cap B  : " << data -> getOriginalArrivalB()[2] << '\n';
    // std::cout << "the cap Zn : " << data -> getOriginalArrivalZn()[2] << '\n';

    // std::cout << "the cap A  : " << data -> getOriginalArrivalA()[3] << '\n'; // the Zn cap is 0 i thought it would be same for A,B
    // std::cout << "the cap B  : " << data -> getOriginalArrivalB()[3] << '\n';
    // std::cout << "the cap Zn : " << data -> getOriginalArrivalZn()[3] << '\n';

    setAnnotationArray(data->getZn(), data->getModifiedArrivalZn(), data->getOriginalArrivalZn());
    setSlew(data->getZn(), data->getModifiedSlewZn());
    return true;

}


void StaInterface::hackModelUpdate_in(sta::Vertex *vertex) {
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


void StaInterface::hackModelUpdate_out(sta::Vertex *vertex) {

    // debugCout(vertex);
    const sta::PortDirection *direction = net_netlist_->direction(vertex->pin());
    if (direction->isOutput()) {
        if (vertex->hasFanout()) {
            if (ml_model_->modelAvailable(getGateName(vertex))){
                DataToModel* data = new DataToModel(vertex);
                bool updated = updateAnnotation_out_pin(data);
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
