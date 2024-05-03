#include "stat_reader.hpp"
#include <iomanip>

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    
    std::string command;
    std::string id;
    
    auto space_pos = request.find(' ');
    if (space_pos == request.npos) {
        throw std::invalid_argument("Your request is empty");
    }
    
    command = std::string(request.substr(0, space_pos));
    id = std::string(request.substr(space_pos + 1));
    
    if (command == "Bus") {
        RouteInfo route_info = transport_catalogue.GetRouteInfo(id);
        if (route_info.distance == 0) {
            output << "Bus " << route_info.bus_name << ": not found" << std::endl;
        } else {
            output << "Bus " << route_info.bus_name << ": " << route_info.bus_stops << " stops on route, ";
            output << route_info.bus_unique_stops << " unique stops, ";
            output << route_info.distance << " route length, ";
            output << route_info.curvative << " curvature" << std::endl;
        }
    } else if (command == "Stop") {
        try {
            std::string result;

            std::set<std::string_view> buses = transport_catalogue.GetStopInfo(id);
            for (std::string_view bus : buses) {
                result += " " + std::string(bus);
            }
            output << command << " " << id << ": " << "buses" << result << std::endl;
        } catch (const std::exception& e) {
            output << command << " " << id << ": " << e.what() << std::endl;
        }
    }
    
}
