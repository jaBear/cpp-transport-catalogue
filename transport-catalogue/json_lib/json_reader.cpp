#include "json_reader.hpp"

void JsonReader::AddStop(const json::Dict& object) {
    base_.AddStop(object.at("name").AsString(), object.at("latitude").AsDouble(), object.at("longitude").AsDouble());
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
    base_.AddRoute(object.at("name").AsString(), bus_stops, object.at("is_roundtrip").AsBool());
    if (object.at("name").IsString()) {
        route_list_.emplace_back(object.at("name").AsString());
    }
}

void JsonReader::AddDistance(const json::Dict& object) {
    std::string stop = object.at("name").AsString();
    
    for (auto& stop_distance : object.at("road_distances").AsMap()) {
        std::string second_stop = stop_distance.first;

        double distance = stop_distance.second.AsDouble();
        if (!base_.IsStopAdded(stop) || !base_.IsStopAdded(second_stop)) {
            throw std::invalid_argument("wrong stops");
        }
        base_.AddDistance(stop, second_stop, distance);
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

void JsonReader::RenderMap(json::Document& document) {
    RenderSettings settings = AddSettingsToBase(document);
    MapRenderer map_renderer(settings, route_list_, base_, doc_svg_);
    map_renderer.MakeSVGDocument();
}

void JsonReader::AddRenderedMap(json::Node& map) {
    try {
        std::ostringstream render_result;
        doc_svg_.Render(render_result);
        builder_
            .Key("map").Value(render_result.str())
            .Key("request_id").Value(map.AsMap().at("id").AsInt());
    } catch (const std::exception& e) {
        builder_
            .Key("request_id").Value(map.AsMap().at("id").AsInt())
            .Key("error_message").Value("not found");
    }
}

void JsonReader::AddBusToRequest(json::Node& map) {
    try {
        RouteInfo ri = base_.GetRouteInfo(map.AsMap().at("name").AsString());
        if (ri.bus_stops == 0 && ri.distance == 0.0) {
            builder_
                .Key("request_id").Value(map.AsMap().at("id").AsInt())
                .Key("error_message").Value("not found");
        }
        builder_
            .Key("curvature").Value(ri.curvative)
            .Key("request_id").Value(map.AsMap().at("id").AsInt())
            .Key("route_length").Value(ri.distance)
            .Key("stop_count").Value(static_cast<int>(ri.bus_stops))
            .Key("unique_stop_count").Value(static_cast<int>(ri.bus_unique_stops));
    } catch (const std::exception& e) {
        builder_
            .Key("request_id").Value(map.AsMap().at("id").AsInt())
            .Key("error_message").Value("not found");
    }
}

void JsonReader::AddStopToRequest(json::Node& map) {
    try {
        std::set<std::string_view> buses = base_.GetStopInfo(map.AsMap().at("name").AsString());
        json::Array array_of_buses;
        for (std::string_view bus : buses) {
            array_of_buses.push_back(std::string{bus});
        }
        builder_
            .Key("buses").Value(array_of_buses)
            .Key("request_id").Value(map.AsMap().at("id").AsInt());
    } catch (const std::exception& e) {
        builder_
            .Key("request_id").Value(map.AsMap().at("id").AsInt())
            .Key("error_message").Value("not found");
    }
}



void JsonReader::ExecuteStatRequest(json::Array& array, std::ostream& out_) {
    builder_.StartArray();
    for (auto& value : array) {
        builder_.StartDict();
        if (value.AsMap().at("type").AsString() == "Map")
            AddRenderedMap(value);
        if (value.AsMap().at("type").AsString() == "Bus")
            AddBusToRequest(value);
        if (value.AsMap().at("type").AsString() == "Stop")
            AddStopToRequest(value);
        builder_.EndDict();
    }
    builder_.EndArray();
    json::Document doc{builder_.Build()};
    json::Print(doc, out_);
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

    
bool JsonReader::ExecuteStatRequestToOut(json::Document& document, std::ostream& out) {
    try {
        const json::Node& node = document.GetRoot();

        if (node.IsMap()) {
            json::Array stat_request = node.AsMap().at("stat_requests").AsArray();
            ExecuteStatRequest(stat_request, out);
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

void JsonReader::AddColor(RenderSettings& settings, const json::Array& value) {
    if (value.size() == 4) {
        svg::Rgba color_scheme{static_cast<uint8_t>(value[0].AsInt()), static_cast<uint8_t>(value[1].AsInt()), static_cast<uint8_t>(value[2].AsInt()), value[3].AsDouble()};
        settings.color_palette.emplace_back(std::move(color_scheme));
    } else {
        if (value.size() == 3) {
            svg::Rgb color_scheme{static_cast<uint8_t>(value[0].AsInt()), static_cast<uint8_t>(value[1].AsInt()), static_cast<uint8_t>(value[2].AsInt())};
            settings.color_palette.emplace_back(std::move(color_scheme));
        }
    }
}

RenderSettings JsonReader::LoadSettings(const json::Node& node) {
    RenderSettings settings;
    try {
        if (node.IsMap()) {
            for (const auto& value : node.AsMap()) {
                if (value.first == "width") {
                    settings.width = value.second.AsDouble();
                }
                
                if (value.first == "height") {
                    settings.height = value.second.AsDouble();
                }
                
                if (value.first == "padding") {
                    settings.padding = value.second.AsDouble();
                }
                
                if (value.first == "line_width") {
                    settings.line_width = value.second.AsDouble();
                }
                
                if (value.first == "stop_radius") {
                    settings.stop_radius = value.second.AsDouble();
                }
                
                if (value.first == "bus_label_font_size") {
                    settings.bus_label_font_size = value.second.AsInt();
                }
                
                if (value.first == "bus_label_offset") {
                    settings.bus_label_offset.x = value.second.AsArray()[0].AsDouble();
                    settings.bus_label_offset.y = value.second.AsArray()[1].AsDouble();
                }
                
                if (value.first == "stop_label_font_size") {
                    settings.stop_label_font_size = value.second.AsInt();
                }
                
                if (value.first == "stop_label_offset") {
                    settings.stop_label_offset.x = value.second.AsArray()[0].AsDouble();
                    settings.stop_label_offset.y = value.second.AsArray()[1].AsDouble();
                }
                
                if (value.first == "underlayer_color") {
                    if (value.second.IsString()) {
                        settings.underlayer_color = value.second.AsString();
                    } else if (value.second.IsArray()) {
                        if (value.second.AsArray().size() == 4) {
                            svg::Rgba color_scheme{static_cast<uint8_t>(value.second.AsArray().at(0).AsInt()), static_cast<uint8_t>(value.second.AsArray().at(1).AsInt()), static_cast<uint8_t>(value.second.AsArray()[2].AsInt()), value.second.AsArray()[3].AsDouble()};
                            settings.underlayer_color = color_scheme;
                        } else {
                            if (value.second.AsArray().size() == 3) {
                                svg::Rgb color_scheme{static_cast<uint8_t>(value.second.AsArray().at(0).AsInt()), static_cast<uint8_t>(value.second.AsArray().at(1).AsInt()), static_cast<uint8_t>(value.second.AsArray()[2].AsInt())};
                                    settings.underlayer_color = color_scheme;
                            }
                        }
                    }
                }
                if (value.first == "underlayer_width") {
                    settings.underlayer_width = value.second.AsDouble();
                }
                if (value.first == "color_palette") {
                    for (const auto& value : value.second.AsArray()) {
                        if (value.IsString()) {
                            settings.color_palette.push_back(value.AsString());
                        } else  {
                            AddColor(settings, value.AsArray());
                        }
                    }
                }
            }
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return settings;
}

RenderSettings JsonReader::AddSettingsToBase(json::Document& document) {
    try {
        const json::Node& node = document.GetRoot();

        if (node.IsMap()) {
            json::Node render_settings = node.AsMap().at("render_settings");
            return LoadSettings(render_settings);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return RenderSettings{};
}

void JsonReader::Process(std::istream& in_stream, std::ostream& out_stream) {
    
    json::Document document = json::Load(in_stream);
    
    bool base_request_load_success = AddJsonToBase(document);
    route_list_ = GetRouteList();
    if (base_request_load_success) {
        std::sort(route_list_.begin(), route_list_.end(), [](const std::string &a, const std::string &b){
                      return std::lexicographical_compare(a.begin(), a.end(),
                                                          b.begin(), b.end());});
    
    RenderMap(document);
    ExecuteStatRequestToOut(document, out_stream);

    }
}
