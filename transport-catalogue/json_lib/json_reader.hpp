#pragma once

#include <iostream>
#include <sstream>

#include "json.hpp"
#include "transport_catalogue.hpp"
#include "map_renderer.hpp"


class JsonReader {
public:
    JsonReader() = default;
    void Process(std::istream& in_stream, std::ostream& out_stream);
private:
    TransportCatalogue base_;
    std::shared_ptr<svg::Document> doc_svg_;
    std::vector<std::string> route_list_;
    
    
    void AddColor(RenderSettings& settings, const json::Array& value);
    
    RenderSettings LoadSettings(const json::Node& node);
    RenderSettings AddSettingsToBase(json::Document& document);
    svg::Document RenderMap(json::Document& document);

    void AddStop(const json::Dict& object);
    void AddBus(const json::Dict& object);
    void AddDistance(const json::Dict& object);
    void AddBaseRequest(json::Array& array);
    std::vector<std::string> GetRouteList();

    bool ExecuteStatRequestToOut(json::Document& document, std::ostream& out, std::shared_ptr<svg::Document> doc);
    
    std::optional<int> GetMapRequestID(json::Array& array);
    
    json::Dict AddRenderedMap(json::Node& map);
    json::Dict AddBusToRequest(json::Node& map);
    json::Dict AddStopToRequest(json::Node& map);
    
    void ExecuteStatRequest(json::Array& node, std::ostream& out_);
    bool AddJsonToBase(json::Document& document);

};
