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
    
    template<typename StringStringViewVector>
    void AddRoute(const std::string& name, StringStringViewVector& route, bool is_circle) {
        if (busname_to_bus_.count(name)) {
            throw std::invalid_argument(name);
        } else {
            Bus new_bus;
            new_bus.name = std::move(name);
            new_bus.circle = is_circle;
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



