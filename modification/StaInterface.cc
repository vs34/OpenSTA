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

void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation) {
    float* arrivals = sta_graph_->arrivals(vertex);
    if (arrivals != nullptr) {
        arrivals = new_annotation;  // Modify the first arrival
    } else {
        std::cerr << "Error: arrivals() returned nullptr!" << std::endl;
    }
}

void StaInterface::updateGraph() {
    for (int index = 1 ; index <= sta_graph_->vertexCount() ; index++) {
        // Check if the vertex is unused

        sta::Vertex *vertex = sta_graph_->vertex(index);
        sta::VertexInEdgeIterator edge_iter(vertex, sta_graph_);
        

        if ((vertex == nullptr) || 
            (vertex->arrivals() == nullptr)) {
            continue;
        }

        std::cout << "portname " <<  net_netlist_->portName(vertex->pin()) << std::endl;
        std::cout << "name " <<  net_netlist_->name(vertex->pin()) << std::endl;
        sta::Instance *instance = net_netlist_->instance(vertex->pin());
        sta::Cell *cell = net_netlist_->cell(instance);
        std::cout << "gate name " << net_netlist_->name(cell) << std::endl;

        std::cout << "vertex->name " <<  vertex->name(net_netlist_) << std::endl;
        std::cout << "this pin " << net_netlist_->pathName(vertex->pin()) << " index " <<index << " " << vertex <<  std::endl;

        while(edge_iter.hasNext()){
            sta::Edge *next_edge = edge_iter.next();
            sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        std::cout << "prev pin " << net_netlist_->pathName(prev_vertex->pin()) << " index " <<index << " " << prev_vertex <<  std::endl;
        }
        float* annotation = getAnnotationArray(vertex);
        if (annotation == nullptr) {
            std::cerr << "Warning: Null annotation at index " << index << std::endl;
            continue;
        }

        /*
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

