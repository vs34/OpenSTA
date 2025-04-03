#include "StaInterface.hh"
#include "MlModel.hh"
#include <iostream>

StaInterface::StaInterface(sta::Graph* graph, sta::Network* network) :
    sta_graph_(graph),
    net_netlist_(network),
    ml_model_(MlModel::getInstance()) 
{}

float* StaInterface::getAnnotationArray(sta::Vertex *vertex) {
    return sta_graph_->arrivals(vertex);
}

float* StaInterface::getSlew(sta::Vertex *vertex) {
        return vertex->slews();
}

float StaInterface::getLoadCapacitance(sta::Pin *pin){
    sta::LibertyPort *lib_port = net_netlist_->libertyPort(pin);
    return lib_port->capacitance();

}

void StaInterface::debugCout(sta::Vertex *vertex) {

    std::cout << "this pin " << net_netlist_->pathName(vertex->pin()) << std::endl;
    //std::cout << "Load Cap " << getLoadCapacitance(vertex->pin()) << std::endl;
    sta::Instance *instance = net_netlist_->instance(vertex->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    std::cout << "gate name " << net_netlist_->name(cell) << std::endl;
}

void StaInterface::updateAnnotation(sta::Vertex *vertex) {
    // if current vertex is INPUT vertex return true
    if (!vertex->hasFanin()){visitedDP[vertex] = true;return;}
    // else if current vertex is OUTPUT vertex copy the arrival of previous to this
    if (!vertex->hasFanout())
        return;
    // else pass the annotation of previous pin
        // to the model to update the annotation
        // then set output annotation to the graph using setAnnotationArray
    visitedDP[vertex] = true;

}


void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation) {
    float* arrivals = sta_graph_->arrivals(vertex);
    if (arrivals != nullptr) {
        arrivals = new_annotation;  // Modify the first arrival
    } else {
        std::cerr << "Error: arrivals() returned nullptr!" << std::endl;
    }
}

void StaInterface::recursiveUpdate(sta::Vertex *vertex) {

    debugCout(vertex);
        if (visitedDP[vertex])
            return;

        sta::VertexInEdgeIterator edge_iter(vertex, sta_graph_);
        
        while(edge_iter.hasNext()){
            sta::Edge *next_edge = edge_iter.next();
            sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
            recursiveUpdate(prev_vertex);
        }

        updateAnnotation(vertex);
        return;
}

void StaInterface::updateGraph() {
    
    for (unsigned int index = 1 ; index <= sta_graph_->vertexCount() ; index++)
        visitedDP[sta_graph_->vertex(index)] = false;

    for (unsigned int index = 1 ; index <= sta_graph_->vertexCount() ; index++) {
        // Check if the vertex is unused

        sta::Vertex *vertex = sta_graph_->vertex(index);

        /*
        if ((vertex == nullptr) || 
            (vertex->arrivals() == nullptr)) {
            continue;
        }
        */
        std::cout << "recursiveUpdate for " << net_netlist_->pathName(vertex->pin()) << std::endl;
        debugCout(vertex);
        recursiveUpdate(vertex);

        /*
        std::cout << "portname " <<  net_netlist_->portName(vertex->pin()) << std::endl;
        std::cout << "name " <<  net_netlist_->name(vertex->pin()) << std::endl;
        sta::Instance *instance = net_netlist_->instance(vertex->pin());
        sta::Cell *cell = net_netlist_->cell(instance);
        std::cout << "gate name " << net_netlist_->name(cell) << std::endl;

        std::cout << "vertex->name " <<  vertex->name(net_netlist_) << std::endl;
        std::cout << "this pin " << net_netlist_->pathName(vertex->pin()) << " index " <<index << " " << vertex <<  std::endl;
        

        float* annotation = getAnnotationArray(vertex);
        if (annotation == nullptr) {
            std::cerr << "Warning: Null annotation at index " << index << std::endl;
            continue;
        }

        
        std::cout << "Annotation: (index " << index << ") "
                  << annotation[0] << ' ' 
                  << annotation[1] << ' ' 
                  << annotation[2] << ' ' 
                  << annotation[3] << ' ' 
                  << annotation[4] << ' ' 
                  << annotation[5] << std::endl;
        
        // Predict new annotation using ML Model
        float updated_value = ml_model_.predict(annotation);
        annotation[0] = updated_value; // Updating the first value as an example

        std::cout << "start pin " << net_netlist_->pathName(vertex->pin()) << std::endl;
        std::cout << "Uudated" << updated_value << std::endl;
        setAnnotationArray(vertex, annotation);
        */
    }
}

