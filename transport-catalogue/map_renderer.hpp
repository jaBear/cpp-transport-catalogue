#pragma once

#include <memory>
#include <variant>

#include "geo.hpp"
#include "svg.hpp"
#include "json.hpp"
#include "transport_catalogue.hpp"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    
    explicit MapRenderer() = default;
    
    void AddColor(std::shared_ptr<RenderSettings>& settings, const json::Array& value) {
        if (value.size() == 4) {
            svg::Rgba color_scheme{static_cast<uint8_t>(value[0].AsInt()), static_cast<uint8_t>(value[1].AsInt()), static_cast<uint8_t>(value[2].AsInt()), value[3].AsDouble()};
            settings->color_palette.emplace_back(std::move(color_scheme));
        } else {
            if (value.size() == 3) {
                svg::Rgb color_scheme{static_cast<uint8_t>(value[0].AsInt()), static_cast<uint8_t>(value[1].AsInt()), static_cast<uint8_t>(value[2].AsInt())};
                settings->color_palette.emplace_back(std::move(color_scheme));
            }
        }
       
    }
    
    std::shared_ptr<RenderSettings> LoadSettings(const json::Node& node) {
        try {
            if (node.IsMap()) {
                for (const auto& value : node.AsMap()) {
                    if (value.first == "width") {
                        settings_->width = value.second.AsDouble();
                    }
                    
                    if (value.first == "height") {
                        settings_->height = value.second.AsDouble();
                    }
                    
                    if (value.first == "padding") {
                        settings_->padding = value.second.AsDouble();
                    }
                    
                    if (value.first == "line_width") {
                        settings_->line_width = value.second.AsDouble();
                    }
                    
                    if (value.first == "stop_radius") {
                        settings_->stop_radius = value.second.AsDouble();
                    }
                    
                    if (value.first == "bus_label_font_size") {
                        settings_->bus_label_font_size = value.second.AsInt();
                    }
                    
                    if (value.first == "bus_label_offset") {
                        settings_->bus_label_offset.x = value.second.AsArray()[0].AsDouble();
                        settings_->bus_label_offset.y = value.second.AsArray()[1].AsDouble();
                    }
                    
                    if (value.first == "stop_label_font_size") {
                        settings_->stop_label_font_size = value.second.AsInt();
                    }
                    
                    if (value.first == "stop_label_offset") {
                        settings_->stop_label_offset.x = value.second.AsArray()[0].AsDouble();
                        settings_->stop_label_offset.y = value.second.AsArray()[1].AsDouble();
                    }
                    
                    if (value.first == "underlayer_color") {
                        if (value.second.IsString()) {
                            settings_->underlayer_color = value.second.AsString();
                        } else if (value.second.IsArray()) {
                            if (value.second.AsArray().size() == 4) {
                                svg::Rgba color_scheme{static_cast<uint8_t>(value.second.AsArray().at(0).AsInt()), static_cast<uint8_t>(value.second.AsArray().at(1).AsInt()), static_cast<uint8_t>(value.second.AsArray()[2].AsInt()), value.second.AsArray()[3].AsDouble()};
                                settings_->underlayer_color = color_scheme;
                            } else {
                                if (value.second.AsArray().size() == 3) {
                                    svg::Rgb color_scheme{static_cast<uint8_t>(value.second.AsArray().at(0).AsInt()), static_cast<uint8_t>(value.second.AsArray().at(1).AsInt()), static_cast<uint8_t>(value.second.AsArray()[2].AsInt())};
                                        settings_->underlayer_color = color_scheme;
                                }
                            }
                        }
                    }
                        
                    
                    
                    if (value.first == "underlayer_width") {
                        settings_->underlayer_width = value.second.AsDouble();
                    }
                    if (value.first == "color_palette") {
                        for (const auto& value : value.second.AsArray()) {
                            if (value.IsString()) {
                                settings_->color_palette.push_back(value.AsString());
                            } else  {
                                AddColor(settings_, value.AsArray());
                            }
                        }
                    }
                }
            }
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        return settings_;
    }
    
    std::shared_ptr<RenderSettings> AddSettingsToBase(json::Document& document) {
        try {
            const json::Node& node = document.GetRoot();

            if (node.IsMap()) {
                json::Node render_settings = node.AsMap().at("render_settings");
                return LoadSettings(render_settings);
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        return nullptr;
    }
    
    
    svg::Document MakeSVGDocument(std::shared_ptr<RenderSettings> settings, std::vector<std::string>& route_list_, std::shared_ptr<TransportCatalogue> base) {
        svg::Document new_document;
        std::vector<svg::shapes::Text> text;
        std::vector<svg::shapes::Text> stop_names;
        std::vector<svg::shapes::Circle> circles;
        


        std::vector<Coordinates> coordinates;
        std::vector<Stop*> stops;
            for (std::string route : route_list_) {
                const Bus* bus = base->GetBusByName(route);
                for (auto& stop : bus->unique_bus_stops) {
                    if (!count(stops.begin(), stops.end(), stop)) {
                        stops.emplace_back(stop);
                    }
                }
            }
        std::sort(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) {
            return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(),
                                                rhs->name.begin(), rhs->name.end());});
//        coordinates.push_back(stop->coordinates);
        for (auto& stop : stops) {
            coordinates.push_back(stop->coordinates);
        }

            
            const SphereProjector proj{
                coordinates.begin(), coordinates.end(), settings->width, settings->height, settings->padding
                };
                
            int count = 0;
            for (std::string route : route_list_) {
                if (static_cast<int>(settings->color_palette.size()) <= count) {
                    count = 0;
                }
                std::string route_name = route;
                const Bus* bus = base->GetBusByName(route_name);
            
                svg::shapes::Polyline svg_route;
                auto copy_of_bus_stops = bus->bus_stops;
                copy_of_bus_stops.pop_back();
                std::reverse(copy_of_bus_stops.begin(), copy_of_bus_stops.end());
                
                if (!bus->circle) {
                    for (auto& stop : bus->bus_stops) {
                        svg::Color new_color{settings->color_palette.at(count)};
                        svg_route.AddPoint(proj(stop->coordinates)).SetFillColor({"none"}).SetStrokeColor(new_color).SetStrokeWidth(settings->line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                    }
//                    for (auto& stop : copy_of_bus_stops) {
//                        svg_route.AddPoint(proj(stop->coordinates));
//                    }
                } else {
                    for (auto& stop : bus->bus_stops) {
                        svg::Color new_color{settings->color_palette.at(count)};
                        svg_route.AddPoint(proj(stop->coordinates)).SetFillColor({"none"}).SetStrokeColor(new_color).SetStrokeWidth(settings->line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                    }
                }
                
                
                
                
                if (!bus->bus_stops.empty()) {
                    svg::shapes::Text svg_text;
                    svg::shapes::Text svg_text2;
                    svg::shapes::Text svg_text3;
                    svg::shapes::Text svg_text4;


                    if (bus->circle) {
                        svg_text.SetFillColor(settings->underlayer_color).SetStrokeColor(settings->underlayer_color).SetStrokeWidth(settings->underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                        svg::Color new_color{settings->color_palette.at(count)};
                        svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                        text.push_back(svg_text);
                        text.push_back(svg_text2);
                    }
                    else {

                            svg::Color new_color{settings->color_palette.at(count)};
                            
                        int last_bus_place = (static_cast<int>(bus->bus_stops.size()) / 2);
                        if (bus->bus_stops.size() > 1) {
                            
                            if (bus->bus_stops.front()->coordinates == bus->bus_stops.at(last_bus_place)->coordinates) {
                                svg_text.SetFillColor(settings->underlayer_color).SetStrokeColor(settings->underlayer_color).SetStrokeWidth(settings->underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                                svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                                text.push_back(svg_text);
                                text.push_back(svg_text2);
                            }
                                else
                            {
                                
                                svg_text.SetFillColor(settings->underlayer_color).SetStrokeColor(settings->underlayer_color).SetStrokeWidth(settings->underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                                svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                                
                                svg_text3.SetFillColor(settings->underlayer_color).SetStrokeColor(settings->underlayer_color).SetStrokeWidth(settings->underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(bus->bus_stops.at(last_bus_place)->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                                svg_text4.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.at(last_bus_place)->coordinates)).SetOffset(settings->bus_label_offset).SetFontSize(settings->bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                            
                                text.push_back(svg_text);
                                text.push_back(svg_text2);
                                text.push_back(svg_text3);
                                text.push_back(svg_text4);
                            }
                        }
                        
                            
//                        }
                    }
                    count++;
                }
                new_document.Add(std::move(svg_route));
            }
        for (auto& stop : stops) {
            svg::shapes::Circle circle;
            circle.SetCenter(proj(stop->coordinates)).SetRadius(settings->stop_radius).SetFillColor("white");
            circles.emplace_back(circle);
        }
        
        for (auto& stop : stops) {
            svg::shapes::Text stop_name;
            svg::shapes::Text stop_name2;

            stop_name.SetFillColor(settings->underlayer_color).SetStrokeColor(settings->underlayer_color).SetStrokeWidth(settings->underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(stop->coordinates)).SetOffset(settings->stop_label_offset).SetFontSize(settings->stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name);
            stop_name2.SetFillColor("black").SetPosition(proj(stop->coordinates)).SetOffset(settings->stop_label_offset).SetFontSize(settings->stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name);
            stop_names.emplace_back(stop_name);
            stop_names.emplace_back(stop_name2);

        }
        
        
        for (svg::shapes::Text& elem : text) {
            new_document.Add(elem);
        }
        
        for (svg::shapes::Circle& elem : circles) {
            new_document.Add(elem);
        }
        
        for (svg::shapes::Text& elem : stop_names) {
            new_document.Add(elem);
        }
        
        return new_document;
   
        
    }
    
    
private:
    std::shared_ptr<RenderSettings> settings_ = std::make_shared<RenderSettings>(RenderSettings{});
};

