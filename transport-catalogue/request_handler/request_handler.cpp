#include "request_handler.hpp"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

//std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const {
//    
//}

void RequestHandler::LaunchBase(std::istream& in_stream, std::ostream& out_stream) {
    base_ = std::make_shared<TransportCatalogue>();
    
    json::Document document = Load(in_stream);
    
    JsonReader reader(base_);
    bool base_request_load_success = reader.AddJsonToBase(document);
    route_list_ = reader.GetRouteList();
    if (base_request_load_success) {
        std::sort(route_list_.begin(), route_list_.end(), [](const std::string &a, const std::string &b){
                      return std::lexicographical_compare(a.begin(), a.end(),
                                                          b.begin(), b.end());});

    std::shared_ptr<svg::Document> doc_svg = std::make_shared<svg::Document>(RenderMap(document));


    
    
    
    reader.ExecuteStatRequestToOut(document, out_stream, doc_svg);

    }
}

