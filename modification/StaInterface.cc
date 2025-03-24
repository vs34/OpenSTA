#include "StaInterface.hh"
#include "MlModel.hh"
#include <iostream>

StaInterface::StaInterface(sta::Graph* graph) : sta_graph_(graph), ml_model_(MlModel::getInstance()) {}

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
        if ((sta_graph_->vertex(index) == nullptr) || 
            (sta_graph_->vertex(index)->arrivals() == nullptr)) {
            continue;
        }

        float* annotation = getAnnotationArray(sta_graph_->vertex(index));
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

        std::cout << "Uudated" << updated_value << std::endl;
        setAnnotationArray(sta_graph_->vertex(index), annotation);
    }
}

