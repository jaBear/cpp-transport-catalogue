#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "json_lib/json_builder.h"

//Метод поиска пути в самом низу FindDistance в приватной части

using namespace graph;

const double meters_in_one_km = 1000.0;
const double minutes_in_hour = 60.0;


class TransportRouter {
public:
    explicit TransportRouter(RouteSettings& route_settings, TransportCatalogue& base) : graph_{new DirectedWeightedGraph<double>{base.GetAllStops().size() * 2}}, route_settings_(route_settings), base_(base){}
    
    friend class JsonReader;
    
private:
    DirectedWeightedGraph<double>* graph_ = nullptr;
    RouteSettings& route_settings_;
    TransportCatalogue& base_;
    std::vector<std::string> route_list_;
    
    void AddEdge(size_t from, size_t to, int from_in_vector, int to_in_vector, const std::vector<Stop*>& stops, std::string bus, int span_count);
    
    void AddRouteList(std::vector<std::string> route_list) {
        route_list_ = route_list;
    }
    
    DirectedWeightedGraph<double>* GetGraphPTR() const{
        return graph_;
    }
    
    void InitiaizeGraph(size_t size_of_stops, const std::deque<Stop>& stops);

    
    double FindTime(double distance, double velocity) const;
    double FindDistance(int from, int to, const std::vector<Stop*>& stops) const;
    
};
