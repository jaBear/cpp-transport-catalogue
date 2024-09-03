#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "json_lib/json_builder.h"


using namespace graph;

const double meters_in_one_km = 1000.0;
const double minutes_in_hour = 60.0;

class InitializedGraph {
public:
    
    explicit InitializedGraph(RouteSettings& route_settings, TransportCatalogue& base, std::vector<std::string> route_list) : graph_{new DirectedWeightedGraph<double>{base.GetAllStops().size() * 2}}, route_settings_(route_settings), base_(base), route_list_(route_list){
        InitiaizeGraph();
    }
    
    DirectedWeightedGraph<double>* GetGraphPTR() const{
        return graph_;
    }
private:
    DirectedWeightedGraph<double>* graph_ = nullptr;
    RouteSettings& route_settings_;
    TransportCatalogue& base_;
    std::vector<std::string> route_list_;
    
    void AddEdge(size_t from, size_t to, int from_in_vector, int to_in_vector, const std::vector<Stop*>& stops, std::string bus, int span_count);
    
    void AddRouteList(std::vector<std::string> route_list) {
        route_list_ = route_list;
    }
    
    
    void InitiaizeGraph();

    double FindTime(double distance, double velocity) const;
    double FindDistance(int from, int to, const std::vector<Stop*>& stops) const;
    
};

class TransportRouter {
public:
    explicit TransportRouter(RouteSettings& route_settings, TransportCatalogue& base, std::vector<std::string> route_list) : graph_{route_settings, base, route_list} {
        router_ = new Router<double>(*graph_.GetGraphPTR());
    }
        
    std::pair<std::vector<Edge<double>>, double> GetEdges(const Stop* stop_from, const Stop* stop_to) {
        auto route_info = router_->BuildRoute(stop_from->edge_id, stop_to->edge_id);
        auto edges = route_info->edges;
        std::vector<Edge<double>> result;
        
        for (auto edge : edges) {
            result.push_back(GetEdgeInfo(edge));
        }
        return std::pair{result, route_info->weight};
    }
    
//
private:
    
    Edge<double> GetEdgeInfo(EdgeId edge_id) {
        return graph_.GetGraphPTR()->GetEdge(edge_id);
    }
    
    Router<double>* router_ = nullptr;
    InitializedGraph graph_;
};

