#pragma once

#include <iostream>
#include <sstream>

#include "json.hpp"
#include "transport_catalogue.hpp"
#include "map_renderer.hpp"

//Print visitor for transport catalogue
struct TransportCataloguePrinter {
    std::ostream& out;
    TransportCatalogue& catalogue;
    json::PrintContext space{out};
    void operator()(std::nullptr_t) const {
        out << "null";
    }
    
    void operator()(json::Array array) const {
        out << "[" << '\n';
        bool is_first = true;
        for (auto& elem : array) {
            if (is_first) {
                std::visit(TransportCataloguePrinter{out, catalogue, space}, elem.GetValue());
                is_first = false;
            } else {
                out << ",";
                std::visit(TransportCataloguePrinter{out, catalogue, space}, elem.GetValue());
            }
        }
        out << "]" << '\n';
    }
    
    void operator()(json::Dict dict) const {
        space.PrintIndent();
        out << "{" << '\n' << "  ";
        bool is_first = true;
        for (auto& elem : dict) {
            if (!is_first) {
                out << ",";
            }
            out << "\"" << elem.first << "\"" << ":";
            std::visit(TransportCataloguePrinter{out, catalogue, space.Indented()}, elem.second.GetValue());
            is_first = false;
            out << '\n';
        }
        out << "}" << '\n';
    }
    
    void operator()(bool val) const {
        out << std::boolalpha << val;
    }
    
    void operator()(int val) const {
        out << val;
//        PrintValue(val, out);
    }
    
    void operator()(double val) const {
        out << val;
//        PrintValue(val, out);
    }
    
    void operator()(std::string val) const {
        json::PrintString(val, out);
    }
};

class JsonReader {
public:
    explicit JsonReader(std::shared_ptr<TransportCatalogue> base) : base_(base) {};
    
    void AddStop(const json::Dict& object) const;
    
    void AddBus(const json::Dict& object);
    
    void AddDistance(const json::Dict& object);
    
    void AddBaseRequest(json::Array& array);
    
    svg::Document RenderMap(json::Document& document);
    
    std::optional<int> GetMapRequestID(json::Array& array);
    
    void LoadStatRequest(json::Array& array, std::ostream& out_);
        
    bool AddJsonToBase(json::Document& document);
    
    bool ExecuteStatRequestToOut(json::Document& document, std::ostream& out, std::shared_ptr<svg::Document> doc);
    
    std::vector<std::string> GetRouteList();

private:
    std::shared_ptr<TransportCatalogue> base_;
    std::shared_ptr<svg::Document> doc_svg_;
    std::vector<std::string> route_list_;
};
