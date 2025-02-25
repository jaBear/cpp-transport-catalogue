#pragma once

#include <memory>
#include <variant>

#include "geo.h"
#include "svg_lib/svg.h"
#include "json_lib/json.h"
#include "transport_catalogue.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};



struct RenderSettings {
    RenderSettings() = default;
    RenderSettings(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size, svg::Point bus_label_offset, int stop_label_font_size, svg::Point stop_label_offset, double underlayer_width) : width(width), height(height), padding(padding), line_width(line_width), stop_radius(stop_radius), bus_label_font_size(bus_label_font_size), bus_label_offset(bus_label_offset), stop_label_font_size(stop_label_font_size), stop_label_offset(stop_label_offset), underlayer_width(underlayer_width){};
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
    explicit MapRenderer(RenderSettings& settings, std::vector<std::string>& route_list, TransportCatalogue& base, svg::Document& svg_doc) : base_(base), settings_(settings), svg_doc_(svg_doc), route_list_(route_list){};
    
    
    void MakeSVGDocument();
    
private:
    const TransportCatalogue& base_;
    RenderSettings& settings_;
    svg::Document& svg_doc_;
    std::vector<std::string>& route_list_;
    
    template <typename obj>
    void AddObjToDocument(obj& objects) {
        for (auto& elem : objects) {
            svg_doc_.Add(elem);
        }
    }
    
    std::vector<Stop*> GetSortedStops() const;
    void AddSettingsToText(svg::shapes::Text& text, std::string name, svg::Point& p);
    void AddRouteSvgParametrs(std::vector<svg::shapes::Text>& objetcts, const SphereProjector& proj);
    
};
