#include "transport_catalogue.hpp"

void TransportCatalogue::AddStop(const Stop& stop) {
    if (stopname_to_stop_.count(stop.name)) {
        throw std::invalid_argument(stop.name);
    } else {
        stops_.push_back(std::move(stop));
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }
}

void TransportCatalogue::AddRoute(const std::string& name, std::vector<std::string_view> route) {
    if (busname_to_bus_.count(name)) {
        throw std::invalid_argument(name);
    } else {
        Bus new_bus;
        new_bus.name = std::move(name);
        std::string stop_name;
        for (std::string_view stop_name_from_route : route) {
            stop_name = std::string(stop_name_from_route);
            
            if (!std::count(new_bus.unique_bus_stops.begin(), new_bus.unique_bus_stops.end(), stopname_to_stop_.at(stop_name))) {
                new_bus.unique_bus_stops.push_back(stopname_to_stop_.at(stop_name));
                stopname_to_bus[stopname_to_stop_.at(stop_name)].push_back(name);
            }
            new_bus.bus_stops.push_back(stopname_to_stop_.at(stop_name));
        }
        buses_.push_back(std::move(new_bus));
        busname_to_bus_[buses_.back().name] = &buses_.back();
        
    }
}

TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(std::string_view name_of_bus) const {
    std::string name = std::string(name_of_bus);
    double distance = 0.0;
    if (!(busname_to_bus_.count(name))) {
        return {name_of_bus, 0, 0, 0.0};
    } else {
        bool is_first = true;
        Coordinates coor_buffer;
        for (Stop* stop : (busname_to_bus_.at(name) -> bus_stops)) {
            if (is_first) {
                coor_buffer = stop->coordinates;
                is_first = false;
            } else {
                distance += ComputeDistance(coor_buffer, stop->coordinates);
                coor_buffer = stop->coordinates;
            }
        }
    }
    return {name_of_bus, busname_to_bus_.at(name)->bus_stops.size(), busname_to_bus_.at(name)->unique_bus_stops.size(), distance};
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view name_of_stop) const {
    std::string name = std::string(name_of_stop);
    
    if (!(stopname_to_stop_.count(name))) {
        throw std::invalid_argument("not found");
    } else if (!(stopname_to_bus.count(stopname_to_stop_.at(name)))) {
        throw std::invalid_argument("no buses");
    }

    std::set<std::string_view> buses;
    for (std::string_view bus : stopname_to_bus.at(stopname_to_stop_.at(name))) {
        buses.insert(bus);
    }
    
    return buses;
}
