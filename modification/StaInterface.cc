#include "StaInterface.hh"
#include "MlModel.hh"
#include <iostream>

// Constructor: initialize the STA graph and network pointer, and get the singleton MlModel instance.
StaInterface::StaInterface(sta::Graph* graph, sta::Network* network ) :
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

// Return the slew pointer for a vertex.
float* StaInterface::getSlew(sta::Vertex *vertex) {
    return vertex->slews();
}

void StaInterface::setSlew(sta::Vertex *vertex, float *newslew) {
    if (newslew[0] != -1)
        vertex->slews()[0] = newslew[0];

    if (newslew[1] != -1)
        vertex->slews()[1] = newslew[1];
}

// Get the load capacitance for a pin by querying the LibertyPort.
float StaInterface::getLoadCapacitance(sta::Pin *pin) {
    if (pin == nullptr) {
        return -1;
    }

    sta::LibertyPort *lib_port = net_netlist_->libertyPort(pin);
    if (lib_port == nullptr) {
        return -1;
        throw std::runtime_error("Null LibertyPort returned in getLoadCapacitance");
    }

    return lib_port->capacitance();
}

// Print out the direction of a pin.
void StaInterface::pinDirectionCout(sta::Pin *pin) {
    const sta::PortDirection *direction = net_netlist_->direction(pin);
    if (direction->isInput()) {
        std::cout << "This pin is an input.\n";
    } else if (direction->isOutput()) {
        std::cout << "This pin is an output.\n";
    } else if (direction->isBidirect()) {
        std::cout << "This pin is bidirectional.\n";
    } else if (direction->isPower()) {
        std::cout << "This pin is a power pin.\n";
    } else if (direction->isGround()) {
        std::cout << "This pin is a ground pin.\n";
    } else {
        std::cout << "Unknown pin direction.\n";
    }
}

// //Debug function: prints details about the vertex, its pin, and associated gate.
void StaInterface::debugCout(sta::Vertex *vertex) {
    std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "vertex - " << vertex << " pin - " << vertex->pin() << std::endl;
    std::cout << "this pin " << net_netlist_->pathName(vertex->pin()) << std::endl;
    //std::cout << "Load Cap " << getLoadCapacitance(vertex->pin()) << std::endl;
    sta::Instance *instance = net_netlist_->instance(vertex->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    std::cout << "gate name " << net_netlist_->name(cell) << std::endl;
    std::cout << "gate name " << net_netlist_->name(net_netlist_->cell(net_netlist_->instance(vertex->pin()))) << std::endl;

    /*
    float* annotation = getAnnotationArray(vertex);
    std::cout << "Annotation: "
              << annotation[0] << ' ' 
              << annotation[1] << ' ' 
              << annotation[2] << ' ' 
              << annotation[3] << ' ' << std::endl;
    */
    pinDirectionCout(vertex->pin());
}

// Update the annotation for a vertex based on its fanin and, if output, via the ML model// we get new annotation of fanout (ZN pin) from previous pin 
void StaInterface::updateAnnotation_fanout_from_fanin(sta::Vertex *vertex) {
    const sta::PortDirection *direction = net_netlist_->direction(vertex->pin());
    sta::VertexInEdgeIterator edge_iter(vertex, sta_graph_);
    
    // If current vertex is an INPUT vertex, mark it visited and return.
    if (!vertex->hasFanin()) {
        visitedDP[vertex] = true;
        return;
    }
    // Else if current vertex is an OUTPUT vertex, simply return.
    else if (!vertex->hasFanout()) {
        return;
    }
    // If the vertex's pin is an input (assumption: the input pin is driven by only one pin)
    else if (direction->isInput()) {
        // Copy annotation of previous pin to this
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        setAnnotationArray(vertex, getAnnotationArray(prev_vertex));
    } 
    else if (direction->isOutput()) { // send this to model
        std::vector<float*> annotation;
        std::vector<float*> slew;
        // ASSUMPTION: the gate name is obtained from the instance associated with the pin
        float cap;
        cap = getLoadCapacitance(vertex->pin());

        sta::Instance *instance = net_netlist_->instance(vertex->pin());
        sta::Cell *cell = net_netlist_->cell(instance);
        const char* model_to_use =  net_netlist_->name(cell);
        while (edge_iter.hasNext()) {
            sta::Edge *next_edge = edge_iter.next();
            sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
            annotation.push_back(getAnnotationArray(prev_vertex));
            slew.push_back(getSlew(prev_vertex)); // Assuming getSlew returns a pointer to float; dereference it.
        }
        // Call the ML model to get updated annotation.
        auto [ch_anno, ch_slew, ch_cap, up_anno, up_slew, up_cap] = 
            ml_model_.getModelAnnotation(model_to_use, annotation, cap, slew);
    
        if (ch_anno)
             setAnnotationArray(vertex, up_anno);
        // If needed, update load capacitance and slew (assuming functions exist).
        // if (ch_cap)
        //    setLoadCapacitance(vertex->pin(), up_cap);
        if (ch_slew)
            setSlew(vertex, up_slew);
    }
    else {
        std::cout << "some Exception on StaInterface::updateAnnotation" << std::endl;
    }
    // Mark vertex as visited.
    visitedDP[vertex] = true;
}

// Sets the annotation array for a vertex.
// NOTE: This implementation simply reassigns the pointer, so ensure this matches your intended behavior.
void StaInterface::setAnnotationArray(sta::Vertex *vertex, float *new_annotation) {
    float* arrivals = sta_graph_->arrivals(vertex);
    float new_arrivals[4];
    if (arrivals != nullptr) {
        bool anotation_update = false;
        for (int a = 0 ; a < 4 ; a++){
            if (new_annotation[a] != -1){  // this is set my model class if -1 no change
                std::cout << "[[[[[[[[UPDATE]]]]]]]] annotation change "<<arrivals[a]<<" to "<<new_annotation[a]<<std::endl;
                new_arrivals[a] = new_annotation[a];
                anotation_update = true;
            }
            else
                new_arrivals[a] = arrivals[a];
        }
//     vertex->setArrivals(new_arrivals);
    if (anotation_update)
        std::cout << "[UPDATE] annotaion updated for "<< net_netlist_->pathName(vertex->pin()) <<  std::endl;
        
    } else {
        std::cerr << "Error: arrivals() returned nullptr!" << std::endl;
    }
}

// Recursively update vertices in the graph, handling cycles.
void StaInterface::recursiveUpdate(sta::Vertex *vertex) {
    // If the vertex is already being updated in a cycle, return.
    if (cycleDect[vertex])
        return;

    cycleDect[vertex] = true;
    sta::VertexInEdgeIterator edge_iter(vertex, sta_graph_);
        
    while (edge_iter.hasNext()) {
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        //debugCout(prev_vertex);
        recursiveUpdate(prev_vertex);
    }
    updateAnnotation_fanout_from_fanin(vertex);
    return;
}

// Main function to update the entire graph.
void StaInterface::updateGraph() {
    // Initialize visitedDP for all vertices.
    for (unsigned int i = 1; i <= sta_graph_->vertexCount(); i++)
        visitedDP[sta_graph_->vertex(i)] = false;

    // Initialize cycle detection map for all vertices.
    for (unsigned int i = 1; i <= sta_graph_->vertexCount(); i++)
        cycleDect[sta_graph_->vertex(i)] = false;

    // Iterate over all vertices and perform recursive updates.
    for (unsigned int index = 1; index <= sta_graph_->vertexCount(); index++) {
        sta::Vertex *vertex = sta_graph_->vertex(index);

        /*
        if ((vertex == nullptr) || 
            (vertex->arrivals() == nullptr)) {
            continue;
        }
        */
        //std::cout << "==================================================" << std::endl;
        // std::cout<<"recrsivUpd for"<<net_netlist_->pathName(vertex->pin())<<std::endl;
        recursiveUpdate(vertex);

     }
}

void StaInterface::updateAnnotation_fanin_from_fanin(sta::Vertex *fanout){
    sta::VertexInEdgeIterator edge_iter(fanout, sta_graph_);
    std::vector<float*> annotation;
    std::vector<float*> slew;
    std::vector<sta::Vertex*> fanin;
    // ASSUMPTION: the gate name is obtained from the instance associated with the pin
    // float cap;
    // cap = getLoadCapacitance(fanout->pin());
    std::vector<float> cap;
    sta::Instance *instance = net_netlist_->instance(fanout->pin());
    sta::Cell *cell = net_netlist_->cell(instance);
    const char* model_to_use =  net_netlist_->name(cell);
    while (edge_iter.hasNext()) {
        
        sta::Edge *next_edge = edge_iter.next();
        sta::Vertex *prev_vertex = next_edge->from(sta_graph_);
        cap.push_back(getLoadCapacitance(prev_vertex->pin()));
        fanin.push_back(prev_vertex);
        annotation.push_back(getAnnotationArray(prev_vertex));
        slew.push_back(getSlew(prev_vertex)); // Assuming getSlew returns a pointer to float; dereference it.
    }
    // Apdating annotation for pin A
    std::cout << "[fetching] model annotation for " << net_netlist_->pathName(fanout->pin()) << std::endl;
    // Call the ML model to get updated annotation.
    auto [ch_anno, ch_slew, ch_cap, up_anno, up_slew, up_cap] = 
         ml_model_.getModelAnnotation(model_to_use, annotation, cap[0], slew);
    
    if (ch_anno)
        setAnnotationArray(fanin[0], up_anno);
    // If needed, update load capacitance and slew (assuming functions exist).
    // if (ch_cap)
    //    setLoadCapacitance(fanin[0]->pin(), up_cap);
    if (ch_slew)
        setSlew(fanin[0], up_slew);


    // Apdating annotaionn for pin B

    std::swap(annotation[0],annotation[1]);
    std::swap(slew[0],slew[1]);
    // Call the ML model to get updated annotation.
    std::tie(ch_anno, ch_slew, ch_cap, up_anno, up_slew, up_cap) =
            ml_model_.getModelAnnotation(model_to_use, annotation, cap[1], slew);

    
    if (ch_anno)
        setAnnotationArray(fanin[1], up_anno);
    // If needed, update load capacitance and slew (assuming functions exist).
    // if (ch_cap)
    //    setLoadCapacitance(fanin[0]->pin(), up_cap);
    if (ch_slew)
        setSlew(fanin[1], up_slew);


}

void StaInterface::hackModelUpdate(sta::Vertex *vertex){
    // this->debugCout(vertex);


    // std::cout << " [debug] loadcapasitance is " << getLoadCapacitance(vertex->pin()) << std::endl;
    const sta::PortDirection *direction = net_netlist_->direction(vertex->pin());

    if (direction->isOutput()){ 
        const char* pin_name = net_netlist_->pathName(vertex->pin());
        if (vertex->hasFanout()) { // Else it will be OUT pin(fan-out of module netlist)
            updateAnnotation_fanin_from_fanin(vertex);
        }
    }
}

