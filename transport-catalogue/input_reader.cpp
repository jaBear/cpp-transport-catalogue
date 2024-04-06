#include "input_reader.hpp"

#include <algorithm>
#include <vector>
#include <cassert>
#include <iterator>

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}


using ParsedDistances = std::vector<std::pair<std::string, std::string>>;

/**
 * Парсит строку вида "9900m to Rasskazovka, 100m to Marushkino" и возвращает вектор расстояний и остановок
 */

std::string_view DeleteCoordinates(std::string_view str) {
    auto first_comma = str.find(',');
    auto only_stops_distance_info_start = str.find(' ', first_comma + 2);
    std::string_view only_stops_distance;
    if (only_stops_distance_info_start != str.npos) {
        only_stops_distance = str.substr(only_stops_distance_info_start + 1);
    }
    return only_stops_distance;
}

ParsedDistances ParseDistance(std::string_view str) {
    
    ParsedDistances result;
    
    str = DeleteCoordinates(str);
    if (str.empty()) {
        return result;
    }
    do {
        if (str.find(',') == 0) {
            str = str.substr(1);
        }
        str = Trim(str);
        auto stop_name_end = str.find(',') - 1;
        
        if (str.find(',') == str.npos) {
            stop_name_end = str.length();
        } else {
            stop_name_end = str.find(',');
        }
        
        auto distance_end = str.find(' ') - 1;
        auto to_word = str.find_first_not_of(' ', distance_end + 2);
        auto stop_name_begin = str.find_first_not_of(' ', to_word + 2);
        
        std::string distance = std::string(str.substr(0, distance_end));
        std::string stop_name = std::string(str.substr(stop_name_begin, stop_name_end - stop_name_begin));
        
        result.push_back({distance, stop_name});
        str = str.substr(stop_name_end);
        
    } while (str.find(',') != str.npos);
    return result;
}

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

bool IsItCircleRoute(std::string_view route) {
    return route.find('>') == route.npos;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    try {
        for (const CommandDescription& command : commands_) {
            if (command.command == "Stop") {
                Stop new_stop;
                new_stop.name = command.id;
                new_stop.coordinates = ParseCoordinates(command.description);
                catalogue.AddStop(new_stop);
            }
        }
        for (const CommandDescription& command : commands_) {
            if (command.command == "Bus") {
                catalogue.AddRoute(command.id, ParseRoute(command.description), IsItCircleRoute(command.description));
            }
        }
        
        for (const CommandDescription& command : commands_) {
            if (command.command == "Stop") {
                ParsedDistances parsed_distance = ParseDistance(command.description);
                for (std::pair<std::string, std::string> dist_stop : parsed_distance) {
                    catalogue.AddDistance(command.id, dist_stop);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << " is already added" << std::endl;
    }
}

void InputReader::ClearCommands() {
    commands_.clear();
}
