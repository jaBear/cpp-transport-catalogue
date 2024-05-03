#pragma once

#include <algorithm>
#include <memory>
#include <set>

#include "transport_catalogue.hpp"
#include "map_renderer.hpp"
#include "json.hpp"
#include "json_reader.hpp"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
using namespace json;

class RequestHandler {
public:

    explicit RequestHandler() = default;
    
    void LaunchBase(std::istream& in_stream, std::ostream& out_stream);
    // Возвращает информацию о маршруте (запрос Bus)
//    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
//    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    
    
    svg::Document RenderMap(Document& document) {
        MapRenderer map_renderer;
        map_renderer.AddSettingsToBase(document);
        return map_renderer.MakeSVGDocument(map_renderer.AddSettingsToBase(document), route_list_, base_);
    }
    
    // MapRenderer понадобится в следующей части итогового проекта
    
private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    
//    const renderer::MapRenderer& renderer_;
    std::shared_ptr<TransportCatalogue> base_;
    std::vector<std::string> route_list_;
};

