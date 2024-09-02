#include "transport_router.h"

double TransportRouter::FindDistance(int from, int to, const std::vector<Stop*>& stops) const {
    double distance = 0.0;

        for (int i = from; i < to; ++i) {
            std::pair<Stop*, Stop*> pair_of_stops{stops.at(i), stops.at(i + 1)};
            distance += base_.GetDistanceBetweenStops(pair_of_stops);
        }

    return distance;
}

double TransportRouter::FindTime(double distance, double velocity) const {
        double distance_between_two_stops_in_km = distance / meters_in_one_km;
        double time_in_hour = distance_between_two_stops_in_km / velocity;
        return time_in_hour *= minutes_in_hour;
}


void TransportRouter::AddEdge(size_t from, size_t to, int from_in_vector, int to_in_vector, const std::vector<Stop*>& stops, std::string bus, int span_count) {
    using Edge = graph::Edge<double>;
    double weight = FindTime(FindDistance(from_in_vector, to_in_vector, stops), route_settings_.bus_velocity);
    graph_->AddEdge(Edge{from, to, weight, bus, span_count});
};

void TransportRouter::InitiaizeGraph(size_t size_of_stops, const std::deque<Stop>& stops) {
    using Edge = graph::Edge<double>;
    double weight = route_settings_.bus_wait_time * 1.0;

    for (int i = 0; i < size_of_stops; ++i) {
        graph_->AddEdge(Edge{stops[i].edge_id, stops[i].edge_id + size_of_stops, weight});
    }
   
};

    
