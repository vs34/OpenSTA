#include "StaInterface.hh"
#include <iostream>

StaInterface::StaInterface(sta::Graph* graph) : sta_graph_(graph) {}

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


    for (int index = 1 ; index <= sta_graph_->vertexCount() ; index++){
        // Check if the vertex is unused

        if ((sta_graph_->vertex(index) == nullptr) || 
        (sta_graph_->vertex(index)->arrivals() == nullptr)) {
            continue;
        }


        float* annotation = getAnnotationArray(sta_graph_->vertex(index));
        // Print the first four annotation values for debugging
        if (annotation == nullptr) {
            std::cerr << "Warning: Null annotation at index " << index << std::endl;
            continue;
        }
        std::cout << "Annotation: (index" << index << ')'
                  << annotation[0] << ' ' 
                  << annotation[1] << ' ' 
                  << annotation[2] << ' ' 
                  << annotation[3] << ' ' 
                  << annotation[4] << ' ' 
                  << annotation[5] << ' ' 
                  << annotation[6] << ' ' 
                  << annotation[7] << std::endl;
        // testing if we can change protected from pointer
        
        annotation[0] = 0.1234; 
        annotation[1] = 1; 
        annotation[2] = 2; 
        annotation[3] = 3; 
        annotation[4] = 4; 
        annotation[5] = 5; 
        // changing index 6 7 break smothing 'double free or corruption (out) Aborted'

        setAnnotationArray(sta_graph_->vertex(index),annotation);

        setAnnotationArray(sta_graph_->vertex(index),annotation);

        // Predict new annotation using ML Model (if needed)
        // float updated_value = MlModel::getInstance().predict(annotation);
        // setAnnotationPinID(sta_graph_->vertices_.blocks_[0].objects_[index].arrivals_, &updated_value);

    }
}

