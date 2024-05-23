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

#include "domain.hpp"
 

class TransportCatalogue {
    
public:
        
    void AddStop(const std::string& name, double lat, double lng);
    
    void AddRoute(const std::string& name, std::vector<std::string>& route, bool is_circle);
    
    void AddRoute(const std::string& name, std::vector<std::string_view>& route, bool is_circle);
    
    void AddDistance(std::string_view main_stop, const std::pair<std::string, std::string>& to_stop);
    
    RouteInfo GetRouteInfo(std::string_view name_of_bus) const;
    std::set<std::string_view> GetStopInfo(std::string_view name_of_stop) const;
    const Bus* GetBusByName(std::string& bus_name) {
        return busname_to_bus_.at(bus_name);
    }
    
    double GetDistanceBetweenStops(std::pair<Stop*, Stop*> two_stops) const;
    
    
    double GetRouteDistance(Bus* bus) const;
    // Функции для тестов
    // Stops
    const std::string& GetLastAddedStopName() {
        return stops_.back().name;
    }
    const Stop& GetLastAddedStop() {
        return stops_.back();
    }
    size_t GetSizeOfStops() {
        return stops_.size();
    }
    // Buses
    const std::string& GetLastAddedBusName() {
        return buses_.back().name;
    }
    const Bus& GetLastAddedBus() {
        return buses_.back();
    }
    size_t GetSizeOfBuses() {
        return buses_.size();
    }

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    // Контейнер unoredered_map для быстрого поиска остановки по её имени
    std::unordered_map<std::string, Stop*, detail::StopnameHasher> stopname_to_stop_;
    // Контейнер unoredered_map для быстрого поиска автобуса по его имени
    std::unordered_map<std::string, Bus*, detail::RoutesHasher> busname_to_bus_;
    
    std::unordered_map<Stop*, std::vector<std::string>> stopname_to_bus;
    
    std::unordered_map<std::pair<Stop*, Stop*>, double, detail::DistanceHasher> distance_between_stops_;

};



