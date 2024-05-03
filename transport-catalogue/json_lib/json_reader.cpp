#include "json_reader.hpp"

void JsonReader::AddStop(const json::Dict& object) const {
    base_->AddStop(object.at("name").AsString(), object.at("latitude").AsDouble(), object.at("longitude").AsDouble());
}

void JsonReader::AddBus(const json::Dict& object) {
    std::vector<std::string_view> bus_stops;
    if (!object.at("stops").AsArray().empty()) {
        const json::Array& array_of_stops = object.at("stops").AsArray();
        for (auto& stop : array_of_stops) {
            bus_stops.emplace_back(stop.AsString());
        }
    }
    if (!object.at("is_roundtrip").AsBool()) {
        std::vector<std::string_view> bus_stops_copy = bus_stops;
        bus_stops_copy.pop_back();
        std::reverse(bus_stops_copy.begin(), bus_stops_copy.end());
        for (auto& bus_stop : bus_stops_copy) {
            bus_stops.emplace_back(bus_stop);
        }
    }
    base_->AddRoute(object.at("name").AsString(), bus_stops, object.at("is_roundtrip").AsBool());
    if (object.at("name").IsString()) {
        route_list_.emplace_back(object.at("name").AsString());
    }
}

void JsonReader::AddDistance(const json::Dict& object) {
    for (auto& stop_distance : object.at("road_distances").AsMap()) {
        std::pair<std::string, std::string> stops(std::to_string(stop_distance.second.AsDouble()), stop_distance.first);
        base_->AddDistance(object.at("name").AsString(), std::move(stops));
    }
}

void JsonReader::AddBaseRequest(json::Array& array) {
    
    for (const auto& value : array) {
        const json::Dict& object = value.AsMap();
        if (object.at("type").AsString() == "Stop") {
            AddStop(object);
        }
    }
    
    for (const auto& value : array) {
        const json::Dict& object = value.AsMap();
        if (object.at("type").AsString() == "Bus") {
            AddBus(object);
        }
    }
    
    for (const auto& value : array) {
        const json::Dict& object = value.AsMap();
        if (object.at("type").AsString() == "Stop" && object.count("road_distances")) {
            AddDistance(object);
        }
    }
}

//std::optional<int> JsonReader::GetMapRequestID(json::Document& doc) {
//    std::optional<int> id;
//    for (const auto& value : array) {
//        const json::Dict& object = value.AsMap();
//        if (object.at("type").AsString() == "map") {
////            LoadMapRequest(object);
//            id = std::optional<int>(object.at("id").AsInt());
//            return id;
//        }
//    }
//    return id;
//}

svg::Document JsonReader::RenderMap(json::Document& document) {
    MapRenderer map_renderer;
    map_renderer.AddSettingsToBase(document);
    return map_renderer.MakeSVGDocument(map_renderer.AddSettingsToBase(document), route_list_, base_);
}

void JsonReader::LoadStatRequest(json::Array& array, std::ostream& out_) {
    bool is_first2 = true;
    out_ << "[" << std::endl;
    for (auto& value : array) {
        if (is_first2) {
            out_ << "   {" << std::endl;
            is_first2 = false;
        } else {
            out_ << ",   {" << std::endl;

        }
        if (value.AsMap().at("type").AsString() == "Map") {
            try {
                
                out_ << "\"map\": ";
                std::ostringstream result;
                doc_svg_->Render(result);
                std::string str = result.str();
                json::PrintString(str, out_);
                out_ << ",";
                out_ << std::endl;
                out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << '\n';

            

        } catch (const std::exception& e) {
            out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << "," << '\n';
            out_ << "       \"error_message\": \"not found\"" << '\n';
        }
        }

        if (value.AsMap().at("type").AsString() == "Bus") {
            try {
                
            RouteInfo ri = base_->GetRouteInfo(value.AsMap().at("name").AsString());
            if (ri.bus_stops == 0 && ri.distance == 0.0) {
                out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << '\n';
                out_ << "       \"error_message\": \"not found\"" << '\n';
            } else {
                out_ << "       \"curvature\": " << ri.curvative << "," << '\n';
                out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << "," << '\n';
                out_ << "       \"route_length\": " << ri.distance << "," << '\n';
                out_ << "       \"stop_count\": " << ri.bus_stops << "," << '\n';
                out_ << "       \"unique_stop_count\": " << ri.bus_unique_stops << '\n';
            }
            

        } catch (const std::exception& e) {
            out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << "," << '\n';
            out_ << "       \"error_message\": \"not found\"" << '\n';
        }
            
        }
            
        if (value.AsMap().at("type").AsString() == "Stop") {
            try {
                std::string result;
                bool is_first = true;
                std::set<std::string_view> buses = base_->GetStopInfo(value.AsMap().at("name").AsString());
                out_ << "       \"buses\": [" << '\n';
                for (std::string_view bus : buses) {
                    if (is_first) {
                        out_ << "           \"" << bus << "\"" << '\n';
                        is_first = false;
                    } else {
                        out_ << "," << '\n' << "\"" << bus << "\"";
                    }
                }
                out_ << "]," << '\n' << "       \"request_id\": " << value.AsMap().at("id").AsInt() << '\n';
                
            } catch (const std::exception& e) {
                out_ << "       \"request_id\": " << value.AsMap().at("id").AsInt() << "," << '\n';
                out_ << "       \"error_message\": \"not found\"" << '\n';
            }
        }
        out_ << "   }" << std::endl;
    }
    out_ << "]" << std::endl;
}

bool JsonReader::AddJsonToBase(json::Document& document) {
    try {
        const json::Node& node = document.GetRoot();

        if (node.IsMap()) {
            json::Array base_request = node.AsMap().at("base_requests").AsArray();
//                const json::Array& render_settings = node.AsMap().at("render_settings").AsArray();
            AddBaseRequest(base_request);
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return false;
}

    
bool JsonReader::ExecuteStatRequestToOut(json::Document& document, std::ostream& out, std::shared_ptr<svg::Document> doc) {
    doc_svg_ = doc;
    try {
        const json::Node& node = document.GetRoot();

        if (node.IsMap()) {
            json::Array stat_request = node.AsMap().at("stat_requests").AsArray();
            LoadStatRequest(stat_request, out);
            return true;
        }
    
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return false;
}

std::vector<std::string> JsonReader::GetRouteList() {
    return route_list_;
}
