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
        std::string_view bus_name;
        size_t bus_stops;
        size_t bus_unique_stops;
        double coordinates;
        std::tie(bus_name, bus_stops, bus_unique_stops, coordinates) = transport_catalogue.GetRouteInfo(id);
        if (coordinates == 0.0) {
            output << "Bus " << bus_name << ": not found" << std::endl;
        } else {
            output << "Bus " << bus_name << ": " << bus_stops << " stops on route, ";
            output << bus_unique_stops << " unique stops, ";
            output << std::setprecision(6) << coordinates << " route length" << std::endl;
        }
    } else if (command == "Stop") {
        std::string result = transport_catalogue.GetStopInfo(id);
        output << result << std::endl;
    }
    
}

