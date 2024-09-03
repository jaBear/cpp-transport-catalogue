#include "json_reader.h"


void JsonReader::AddStop(const json::Dict& object) {
    base_.AddStop(object.at("name").AsString(), object.at("latitude").AsDouble(), object.at("longitude").AsDouble());
}


void JsonReader::AddBus(const json::Dict& object) {
    
    if (!object.at("stops").AsArray().empty()) {
        
        const json::Array& array_of_stops = object.at("stops").AsArray();
        std::vector<Stop*> stops;
        std::string bus_name = object.at("name").AsString();
        
        for (auto stop_name : array_of_stops) {
            stops.push_back(base_.GetStopByName(stop_name.AsString()));
        }
        std::vector<Stop*> stops_for_routing = stops;

        
        if (!object.at("is_roundtrip").AsBool()) {
            std::vector<Stop*> bus_stops_copy = stops;
            bus_stops_copy.pop_back();
            std::reverse(bus_stops_copy.begin(), bus_stops_copy.end());
            for (auto& bus_stop : bus_stops_copy) {
                stops.push_back(bus_stop);
            }
        }
        
        base_.AddRoute(bus_name, stops, object.at("is_roundtrip").AsBool());
        if (object.at("name").IsString()) {
            route_list_.emplace_back(bus_name);
        }

//        size_t all_stops_size = base_.GetAllStops().size();
//        if (stops.size() > 1) {
//            int span_count = 0;
//            for (int i = 0; i < stops.size() - 1; ++i) {
//                for (int x = i + 1; x < stops.size(); ++x) {
//                    if (stops.at(i) != stops.at(x)) {
//                        router_->AddEdge(stops.at(i)->edge_id + all_stops_size, stops.at(x)->edge_id, i, x, stops, bus_name, ++span_count);
//                    }
//                }
//            }
//        }
        
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
        if (object.at("type").AsString() == "Stop" && object.count("road_distances")) {
            AddDistance(object);
        }
    }
    
    for (const auto& value : array) {
        const json::Dict& object = value.AsMap();
        if (object.at("type").AsString() == "Bus") {
            AddBus(object);
        }
    }
    router_ = new TransportRouter{route_settings_, base_, route_list_};

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

void JsonReader::AddRouteToRequest(json::Node& map) {
    std::deque<Stop> stops = base_.GetAllStops();
    try {
        std::string stop_name_from = map.AsMap().at("from").AsString();
        std::string stop_name_to = map.AsMap().at("to").AsString();

        Stop* stop_from = base_.GetStopByName(stop_name_from);
        Stop* stop_to = base_.GetStopByName(stop_name_to);
        
        std::pair<std::vector<Edge<double>>, double> route_info = router_->GetEdges(stop_from, stop_to);

        if (!route_info.first.empty()) {
            builder_
                .Key("items")
                .StartArray();
            
            int count = 0;
            for (auto& edge : route_info.first) {
                if (count % 2 == 0) {
                std::optional<Stop*> stop_iter = base_.GetStopByEdge(edge.from);
                    std::string stop_name = stop_iter.value()->name;
                    builder_
                        .StartDict()
                        .Key("stop_name")
                        .Value(stop_name)
                        .Key("time")
                        .Value(route_settings_.bus_wait_time)
                        .Key("type")
                        .Value("Wait")
                        .EndDict();
                } else {
                        builder_
                            .StartDict()
                            .Key("bus")
                            .Value(edge.bus)
                            .Key("span_count")
                            .Value(edge.span_count)
                            .Key("time")
                            .Value(edge.weight)
                            .Key("type")
                            .Value("Bus")
                            .EndDict();
                }
                ++count;
                

            }
            
                builder_
                .EndArray()
                .Key("request_id").Value(map.AsMap().at("id").AsInt())
                .Key("total_time").Value(route_info.second);
            
        } else {
            builder_
                .Key("request_id").Value(map.AsMap().at("id").AsInt())
                .Key("error_message").Value("not found");
        }
        
        
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
//    Router router{*router_->GetGraphPTR()};

    for (auto& value : array) {
        builder_.StartDict();
        if (value.AsMap().at("type").AsString() == "Map")
            AddRenderedMap(value);
        if (value.AsMap().at("type").AsString() == "Bus")
            AddBusToRequest(value);
        if (value.AsMap().at("type").AsString() == "Stop")
            AddStopToRequest(value);
        if (value.AsMap().at("type").AsString() == "Route")
            AddRouteToRequest(value);
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
    auto dict = node.AsMap();
        
    
    RenderSettings settings{dict.at("width").AsDouble(), dict.at("height").AsDouble(), dict.at("padding").AsDouble(), dict.at("line_width").AsDouble(), dict.at("stop_radius").AsDouble(), dict.at("bus_label_font_size").AsInt(), svg::Point{dict.at("bus_label_offset").AsArray()[0].AsDouble(), dict.at("bus_label_offset").AsArray()[1].AsDouble()}, dict.at("stop_label_font_size").AsInt(), svg::Point{dict.at("stop_label_offset").AsArray()[0].AsDouble(), dict.at("stop_label_offset").AsArray()[1].AsDouble()}, dict.at("underlayer_width").AsDouble()};
            
                
        if (dict.at("underlayer_color").IsString()) {
            settings.underlayer_color = dict.at("underlayer_color").AsString();
        } else if (dict.at("underlayer_color").IsArray()) {
            if (dict.at("underlayer_color").AsArray().size() == 4) {
                svg::Rgba color_scheme{static_cast<uint8_t>(dict.at("underlayer_color").AsArray().at(0).AsInt()), static_cast<uint8_t>(dict.at("underlayer_color").AsArray().at(1).AsInt()), static_cast<uint8_t>(dict.at("underlayer_color").AsArray()[2].AsInt()), dict.at("underlayer_color").AsArray()[3].AsDouble()};
                settings.underlayer_color = color_scheme;
            } else {
                if (dict.at("underlayer_color").AsArray().size() == 3) {
                    svg::Rgb color_scheme{static_cast<uint8_t>(dict.at("underlayer_color").AsArray().at(0).AsInt()), static_cast<uint8_t>(dict.at("underlayer_color").AsArray().at(1).AsInt()), static_cast<uint8_t>(dict.at("underlayer_color").AsArray()[2].AsInt())};
                        settings.underlayer_color = color_scheme;
                }
            }
        }
                
       
                
    
        for (const auto& value : dict.at("color_palette").AsArray()) {
            if (value.IsString()) {
                settings.color_palette.push_back(value.AsString());
            } else  {
                AddColor(settings, value.AsArray());
            }
        }
        
    return settings;
}

RenderSettings JsonReader::AddSettingsToBase(json::Document& document) {
    try {
        const json::Node& node = document.GetRoot();

            json::Node render_settings = node.AsMap().at("render_settings");
            return LoadSettings(render_settings);
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return RenderSettings{};
}


RouteSettings JsonReader::LoadRouteSettings(const json::Node& node) {
    RouteSettings settings;
    
    try {
            settings.bus_velocity = node.AsMap().at("bus_velocity").AsDouble();
            settings.bus_wait_time = node.AsMap().at("bus_wait_time").AsInt();

    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return settings;
}


RouteSettings JsonReader::AddRouteSettings(json::Document& document) {
    try {
        const json::Node& node = document.GetRoot();

        if (node.IsMap()) {
            json::Node route_settings = node.AsMap().at("routing_settings");
            return LoadRouteSettings(route_settings);
        }
    
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return RouteSettings{};
};






void JsonReader::Process(std::istream& in_stream, [[maybe_unused]] std::ostream& out_stream) {

    json::Document document = json::Load(in_stream);

    route_settings_ = AddRouteSettings(document);
    bool base_request_load_success = AddJsonToBase(document);

    
//    route_list_copy_ = GetRouteList();
    if (base_request_load_success) {
        std::sort(route_list_.begin(), route_list_.end(), [](const std::string &a, const std::string &b){
                      return std::lexicographical_compare(a.begin(), a.end(),
                                                          b.begin(), b.end());});
        route_list_ = GetRouteList();

        
        RenderMap(document);
        ExecuteStatRequestToOut(document, out_stream);
//
    }
}
