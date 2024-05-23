#include "map_renderer.hpp"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void AddSettingsToText(RenderSettings& settings, svg::shapes::Text& text, std::string name, svg::Point& p) {
    text.SetFillColor(settings.underlayer_color).SetStrokeColor(settings.underlayer_color).SetStrokeWidth(settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(p).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(name);
}

void AddRouteSvgParametrs(std::vector<svg::shapes::Text>& objetcts, const SphereProjector& proj, RenderSettings& settings, svg::Document& new_document, std::vector<std::string>& route_list_, std::shared_ptr<TransportCatalogue> base) {

    int count = 0;
    for (std::string route : route_list_) {
        if (static_cast<int>(settings.color_palette.size()) <= count) {
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
                svg::Color new_color{settings.color_palette.at(count)};
                svg_route.AddPoint(proj(stop->coordinates)).SetFillColor({"none"}).SetStrokeColor(new_color).SetStrokeWidth(settings.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            }
        }
        if (!bus->bus_stops.empty()) {
            svg::shapes::Text svg_text;
            svg::shapes::Text svg_text2;
            svg::shapes::Text svg_text3;
            svg::shapes::Text svg_text4;
            svg::Point p = proj(bus->bus_stops.front()->coordinates);
            AddSettingsToText(settings, svg_text, bus->name, p);
        if (bus->circle) {
            svg::Color new_color{settings.color_palette.at(count)};
            svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
            objetcts.push_back(svg_text);
            objetcts.push_back(svg_text2);
        }
        else {
            svg::Color new_color{settings.color_palette.at(count)};
            int last_bus_place = (static_cast<int>(bus->bus_stops.size()) / 2);
            if (bus->bus_stops.size() > 1) {
                if (bus->bus_stops.front()->coordinates == bus->bus_stops.at(last_bus_place)->coordinates) {
                    svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                    objetcts.push_back(svg_text);
                    objetcts.push_back(svg_text2);
                }
                svg_text2.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.front()->coordinates)).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
                
                svg::Point p2 = proj(bus->bus_stops.at(last_bus_place)->coordinates);
                
                AddSettingsToText(settings, svg_text3, bus->name, p2);
                svg_text4.SetFillColor(new_color).SetPosition(proj(bus->bus_stops.at(last_bus_place)->coordinates)).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->name);
            
                objetcts.push_back(svg_text);
                objetcts.push_back(svg_text2);
                objetcts.push_back(svg_text3);
                objetcts.push_back(svg_text4);
            }
        }
    count++;
    }
    new_document.Add(std::move(svg_route));
}
}

std::vector<Stop*> GetSortedStops(std::vector<std::string>& route_list_, std::shared_ptr<TransportCatalogue>& base) {
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
    return stops;
}

template <typename obj>
void AddObjToDocument(svg::Document& doc, obj& objects) {
    for (auto& elem : objects) {
        doc.Add(elem);
    }
}

svg::Document MapRenderer::MakeSVGDocument(RenderSettings& settings, std::vector<std::string>& route_list_, std::shared_ptr<TransportCatalogue> base) {
    
    svg::Document document;
    std::vector<svg::shapes::Text> text;
    std::vector<svg::shapes::Text> stop_names;
    std::vector<svg::shapes::Circle> circles;
    std::vector<Coordinates> coordinates;
    
    std::vector<Stop*> stops = GetSortedStops(route_list_, base);
    
    for (auto& stop : stops) {
        coordinates.push_back(stop->coordinates);
    }
    const SphereProjector proj{
        coordinates.begin(), coordinates.end(), settings.width, settings.height, settings.padding
    };
            
    AddRouteSvgParametrs(text, proj, settings, document, route_list_, base);
    
    for (auto& stop : stops) {
        svg::shapes::Circle circle;
        circle.SetCenter(proj(stop->coordinates)).SetRadius(settings.stop_radius).SetFillColor("white");
        circles.emplace_back(circle);
    }
    
    for (auto& stop : stops) {
        svg::shapes::Text stop_name;
        svg::shapes::Text stop_name2;
        stop_name.SetFillColor(settings.underlayer_color).SetStrokeColor(settings.underlayer_color).SetStrokeWidth(settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(proj(stop->coordinates)).SetOffset(settings.stop_label_offset).SetFontSize(settings.stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name);
        stop_name2.SetFillColor("black").SetPosition(proj(stop->coordinates)).SetOffset(settings.stop_label_offset).SetFontSize(settings.stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name);
        stop_names.emplace_back(stop_name);
        stop_names.emplace_back(stop_name2);
    }
    
    AddObjToDocument(document, text);
    AddObjToDocument(document, circles);
    AddObjToDocument(document, stop_names);

    return document;
}
