#pragma once

#include "geo.hpp"
#include <string>
#include <string_view>
#include <vector>

struct RouteInfo {
    std::string_view bus_name;
    size_t bus_stops;
    size_t bus_unique_stops;
    double distance;
    double curvative;
    bool is_round;
};
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

//Хешеры для структур

namespace detail {
struct StopnameHasher {
    // Временный dummy хешер. Написать корректный
    size_t operator() (const std::string& stop) const {
        return hasher(static_cast<int>(stop[0] * 10 * stop.size() + (stop.back())));
    }
private:
    std::hash<int> hasher;
};

struct RoutesHasher {
    // Временный dummy хешер. Написать корректный
    size_t operator() (const std::string& bus) const {
        return hasher(static_cast<int>(bus[0] * 10 * bus.size() + (bus.back())));
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
