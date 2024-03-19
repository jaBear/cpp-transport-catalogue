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
}
 

class TransportCatalogue {
    
public:
    
    void AddStop(const Stop& stop) {
        if (stopname_to_stop_.count(stop.name)) {
            std::cerr << stop.name << " is already added" << std::endl;
        } else {
            stops_.push_back(std::move(stop));
            stopname_to_stop_[stops_.back().name] = &stops_.back();
        }
    }
    void AddRoute(const std::string& name, std::vector<std::string_view> route) {
        if (busname_to_bus_.count(name)) {
            std::cerr << name << " is already added" << std::endl;
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
    
    
    std::tuple<std::string_view, size_t, size_t, double> GetRouteInfo(std::string_view name_of_bus) const {
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
            
//            std::cout << name << ": " << busname_to_bus_.at(name)->bus_stops.size() << " stops on route, ";
//            std::cout << busname_to_bus_.at(name)->unique_bus_stops.size() << " unique stops, ";
//            std::cout << distance << " route length" << std::endl;
        
        // Функция для получения информации о маршруте, должна возвращать сам маршрут
    }
    
    std::string GetStopInfo(std::string_view name_of_stop) const {
        std::string name = std::string(name_of_stop);
        std::string result = "Stop " + name;
        
        if (!(stopname_to_stop_.count(name))) {
            result += ": not found";
            return result;
        } else if (!(stopname_to_bus.count(stopname_to_stop_.at(name)))) {
            result += ": no buses";
            return result;
        }
        
        result += ": buses";
        std::set<std::string> buses;
        for (std::string bus : stopname_to_bus.at(stopname_to_stop_.at(name))) {
            buses.insert(bus);
        }
        for (std::string bus : buses) {
            result += " " + bus;
        }
        return result;
    }
    
    // Функции для тестов
    // Stops
    std::string GetLastAddedStopName() {
        return stops_.back().name;
    }
    Stop GetLastAddedStop() {
        return stops_.back();
    }
    size_t GetSizeOfStops() {
        return stops_.size();
    }
    // Buses
    std::string GetLastAddedBusName() {
        return buses_.back().name;
    }
    Bus GetLastAddedBus() {
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

};

