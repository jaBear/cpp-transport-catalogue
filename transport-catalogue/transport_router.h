#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "json_lib/json_builder.h"

using namespace graph;

class TransportRouter {
public:
    explicit TransportRouter(RouteSettings& route_settings, TransportCatalogue& base) : graph_{new DirectedWeightedGraph<double>{base.GetAllStops().size() * 2}}, route_settings_(route_settings), base_(base){}

    void AddEdge(size_t from, size_t to, int from_in_vector, int to_in_vector, const std::vector<Stop*>& stops, std::string bus, int span_count);
    
    DirectedWeightedGraph<double>* GetGraphPTR() const{
        return graph_;
    }
    
    void AddRouteList(std::vector<std::string> route_list) {
        route_list_ = route_list;
    }
    
    void InitiaizeGraph(size_t size_of_stops, const std::deque<Stop>& stops);

    
private:
    DirectedWeightedGraph<double>* graph_ = nullptr;
    RouteSettings& route_settings_;
    TransportCatalogue& base_;
    std::vector<std::string> route_list_;
    
    double FindTime(double distance, double velocity) const;
    double FindDistance(int from, int to, const std::vector<Stop*>& stops) const;
    
};
