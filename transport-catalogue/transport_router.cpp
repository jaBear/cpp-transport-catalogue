#include "transport_router.h"

double InitializedGraph::FindDistance(int from, int to, const std::vector<Stop*>& stops) const {
    double distance = 0.0;

        for (int i = from; i < to; ++i) {
            std::pair<Stop*, Stop*> pair_of_stops{stops.at(i), stops.at(i + 1)};
            distance += base_.GetDistanceBetweenStops(pair_of_stops);
        }

    return distance;
}

double InitializedGraph::FindTime(double distance, double velocity) const {
        double distance_between_two_stops_in_km = distance / meters_in_one_km;
        double time_in_hour = distance_between_two_stops_in_km / velocity;
        return time_in_hour *= minutes_in_hour;
}


void InitializedGraph::AddEdge(size_t from, size_t to, int from_in_vector, int to_in_vector, const std::vector<Stop*>& stops, std::string bus, int span_count) {
    using Edge = graph::Edge<double>;
    double weight = FindTime(FindDistance(from_in_vector, to_in_vector, stops), route_settings_.bus_velocity);
    graph_->AddEdge(Edge{from, to, weight, bus, span_count});
};

void InitializedGraph::InitiaizeGraph() {
    using Edge = graph::Edge<double>;
    double weight = route_settings_.bus_wait_time * 1.0;
    const std::deque<Stop>& stops = base_.GetAllStops();
    
    for (int i = 0; i < stops.size(); ++i) {
        graph_->AddEdge(Edge{stops[i].edge_id, stops[i].edge_id + stops.size(), weight});
    }
    
    for (std::string& route : route_list_) {
        const Bus* bus = base_.GetBusByName(route);
        size_t stops_count = bus->bus_stops.size();
        if (stops_count > 1) {
            int span_count = 0;
            for (int i = 0; i < stops_count - 1; ++i) {
                for (int x = i + 1; x < stops_count; ++x) {
                    if (bus->bus_stops.at(i) != bus->bus_stops.at(x)) {
                        AddEdge(bus->bus_stops.at(i)->edge_id + base_.GetAllStops().size(), bus->bus_stops.at(x)->edge_id, i, x, bus->bus_stops, bus->name, ++span_count);
                    }
                }
            }
        }
    }
    
   
};

    
