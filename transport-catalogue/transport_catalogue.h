#pragma once
#include <algorithm>
#include <deque>
#include <string>
#include <stdexcept>
#include <set>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <utility>

#include "domain.h"
 

class TransportCatalogue {
    
public:
        
    void AddStop(const std::string& name, double lat, double lng);
        
    void AddRoute(const std::string& name, const std::vector<Stop*>& route, bool is_circle);
    
    void AddDistance(const std::string& first_stop, const std::string& second_stop, double distance);
    
    bool IsStopAdded(std::string& stop_name);
    
    RouteInfo GetRouteInfo(std::string_view name_of_bus) const;
    std::set<std::string_view> GetStopInfo(std::string_view name_of_stop) const;
    const Bus* GetBusByName(std::string& bus_name) const {
        return busname_to_bus_.at(bus_name);
    }
    
    Stop* GetStopByName(std::string stop_name) const{
        return stopname_to_stop_.at(stop_name);
    }
    
    std::optional<Stop*> GetStopByEdge(size_t edge_id) const{
        return edge_to_stop_.at(edge_id);
    }
    
    double GetDistanceBetweenStops(std::pair<Stop*, Stop*>& two_stops) const;
    bool IsDistanceAdded(std::pair<Stop*, Stop*>& two_stops) const {
        return distance_between_stops_.count(two_stops);
    }
    const std::deque<Stop>& GetAllStops() const;
    std::unordered_map<std::string, Stop*, detail::StopnameHasher> GetAllStopsMap() const;


private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    // Контейнер unoredered_map для быстрого поиска остановки по её имени
    std::unordered_map<std::string, Stop*, detail::StopnameHasher> stopname_to_stop_;
    // Контейнер unoredered_map для быстрого поиска автобуса по его имени
    std::unordered_map<std::string, Bus*, detail::RoutesHasher> busname_to_bus_;
    std::unordered_map<size_t, Stop*, detail::EdgeHasher> edge_to_stop_;

    std::unordered_map<Stop*, std::vector<std::string>> stopname_to_bus;
    
    std::unordered_map<std::pair<Stop*, Stop*>, double, detail::DistanceHasher> distance_between_stops_;
    
    double GetRouteDistance(Bus* bus) const;

};


