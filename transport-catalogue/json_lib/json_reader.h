#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include "../router.h"
#include "../graph.h"
#include "json.h"
#include "../transport_router.h"
#include "../transport_catalogue.h"
#include "../map_renderer.h"
#include "json_builder.h"

class JsonReader {
public:
    JsonReader() = default;
    void Process(std::istream& in_stream, std::ostream& out_stream);
    ~JsonReader() = default;
private:
    TransportCatalogue base_;
    svg::Document doc_svg_;
    json::Builder builder_;
    std::vector<std::string> route_list_;
//    std::vector<std::string> route_list_copy_;

    RouteSettings route_settings_;
    TransportRouter* router_ = nullptr;
    DirectedWeightedGraph<double>* graph_ = nullptr;
    std::map<size_t, Stop*> edge_id_to_stop;
    
    
    void AddColor(RenderSettings& settings, const json::Array& value);
    
    RenderSettings LoadSettings(const json::Node& node);
    RenderSettings AddSettingsToBase(json::Document& document);
    void RenderMap(json::Document& document);
    RouteSettings AddRouteSettings(json::Document& document);
    RouteSettings LoadRouteSettings(const json::Node& node);
    graph::DirectedWeightedGraph<double> BuildGraph();

    
    
    void AddStop(const json::Dict& object);
    void AddBus(const json::Dict& object);
    void AddDistance(const json::Dict& object);
    void AddBaseRequest(json::Array& array);
    std::vector<std::string> GetRouteList();

    bool ExecuteStatRequestToOut(json::Document& document, std::ostream& out);
    
    std::optional<int> GetMapRequestID(json::Array& array);
    
    void AddRenderedMap(json::Node& map);
    void AddBusToRequest(json::Node& map);
    void AddStopToRequest(json::Node& map);
    void AddRouteToRequest(json::Node& map);
    
    void ExecuteStatRequest(json::Array& array, std::ostream& out_);
    bool AddJsonToBase(json::Document& document);

};
