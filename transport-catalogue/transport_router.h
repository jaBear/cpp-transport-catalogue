#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "json_builder.h"

using namespace graph;

class TransportRouter {
public:
    explicit TransportRouter(size_t vertex_count, RouteSettings& route_settings, TransportCatalogue& base) : graph_{vertex_count}, route_settings_(route_settings), base_(base) {}
    
    DirectedWeightedGraph<double>& BuildGraph();
    
private:
    DirectedWeightedGraph<double> graph_;
    RouteSettings& route_settings_;
    TransportCatalogue& base_;
    
};
