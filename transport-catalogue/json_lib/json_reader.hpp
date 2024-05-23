#pragma once

#include <iostream>
#include <sstream>

#include "json.hpp"
#include "transport_catalogue.hpp"
#include "map_renderer.hpp"


class JsonReader {
public:
    JsonReader() = default;
    
    void AddStop(const json::Dict& object) const;
    
    void AddBus(const json::Dict& object);
    
    void AddDistance(const json::Dict& object);
    
    void AddBaseRequest(json::Array& array);
    
    svg::Document RenderMap(json::Document& document);
    
    std::optional<int> GetMapRequestID(json::Array& array);
    
    json::Dict AddRenderedMap(json::Node& map);
    
    json::Dict AddBusToRequest(json::Node& map);
    
    json::Dict AddStopToRequest(json::Node& map);
    
    void ExecuteStatRequest(json::Array& node, std::ostream& out_);
        
    bool AddJsonToBase(json::Document& document);
    
    bool ExecuteStatRequestToOut(json::Document& document, std::ostream& out, std::shared_ptr<svg::Document> doc);
    
    std::vector<std::string> GetRouteList();
    
    void LaunchBase(std::istream& in_stream, std::ostream& out_stream);
    
    void AddColor(RenderSettings& settings, const json::Array& value);
    
    RenderSettings LoadSettings(const json::Node& node);
    
    RenderSettings AddSettingsToBase(json::Document& document);

private:
    std::shared_ptr<TransportCatalogue> base_;
    std::shared_ptr<svg::Document> doc_svg_;
    std::vector<std::string> route_list_;
};
