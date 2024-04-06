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
#include "geo.hpp"


struct Stop {
    // Структура остановки
    // Перегружаем оператор сравнения
    bool operator==(const Stop& other) const {
        return name == other.name && coordinates == other.coordinates;
    }
    
    std::string name;
    Coordinates coordinates;
};

struct Bus {
    // Структура маршрута
    std::string name;
    std::vector<Stop*> bus_stops;
    std::vector<Stop*> unique_bus_stops;
    bool circle;
};

namespace detail {
struct StopnameHasher {
    // Временный dummy хешер. Написать корректный
    size_t operator() (const std::string& stop) const {
        return hasher(stop[0] * 10 * stop.size() + (stop.back()));
    }
private:
    std::hash<int> hasher;
};

struct RoutesHasher {
    // Временный dummy хешер. Написать корректный
    size_t operator() (const std::string& bus) const {
        return hasher(bus[0] * 10 * bus.size() + (bus.back()));
    }
private:
    std::hash<int> hasher;
};

struct DistanceHasher {
    size_t operator() (const std::pair<Stop*, Stop*>& stops) const {
        return hasher(stops.first);
    }
private:
    std::hash<const void*> hasher;
};
}
 

class TransportCatalogue {
    
public:
    struct RouteInfo {
        std::string_view bus_name;
        size_t bus_stops;
        size_t bus_unique_stops;
        double distance;
        double curvative;
    };
    
    void AddStop(const Stop& stop);
    void AddRoute(const std::string& name, std::vector<std::string_view> route, bool circle);
    void AddDistance(std::string_view main_stop, const std::pair<std::string, std::string>& stops);
    
    RouteInfo GetRouteInfo(std::string_view name_of_bus) const;
    std::set<std::string_view> GetStopInfo(std::string_view name_of_stop) const;
    
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



