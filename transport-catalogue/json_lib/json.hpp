#pragma once

#include <iostream>
#include <optional>
#include <map>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    //Visitor для проверки типа значения
    struct NodeChecker {
        std::string& result;
        //Методы NodeChecker(Visitor для variant)
        void operator()(std::nullptr_t) const;
        void operator()(Array array) const;
        void operator()(Dict dict) const;
        void operator()(bool val) const;
        void operator()(int val) const;
        void operator()(double val) const;
        void operator()(std::string val) const;
    };


    //Перегрузка операторов сравнения
    bool operator==(const Node& lhs) const {
        return GetValue() == lhs.GetValue();
    }
    bool operator!=(const Node& lhs) const {
        return !(*this == lhs.GetValue());
    }

    const Value& GetValue() const;
    const std::string GetNodeObj() const;
    
    //Методы Node для проверки типа значения внутри val_
    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;
    
    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;

private:
    Value val_ = nullptr;
};

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using Number = std::variant<int, double>;

std::string LoadString(std::istream& input);

Number LoadNumber(std::istream& input);

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input);

class Document {
public:
    explicit Document(Node root);
    bool operator ==(const Document& rhs);
    bool operator!=(const Document& rhs);
    const Node& GetRoot() const;
    Node& GetRoot();

private:
    Node root_;
};

struct ValChecker {
    std::string& type;
    
    void operator()(int) {
        type = "int";
    }
    void operator()(double) {
        type = "double";
    }
};

Document Load(std::istream& input);

using namespace std::literals;

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

//template <typename Value>
//void PrintValue(const Value& value, std::ostream& out) {
//    out << value;
//}// Перегрузка функции PrintValue для вывода значений null
//void PrintValue(std::nullptr_t, std::ostream& out);
//
//void PrintNode(const Node& node, std::ostream& out);

void PrintString(std::string, std::ostream& out);

struct NodePrinter {
    std::ostream& out;
    PrintContext space{out};
    void operator()(std::nullptr_t) const {
        out << "null";
    }
    
    void operator()(Array array) const {
        out << "[" << '\n';
        bool is_first = true;
        for (auto& elem : array) {
            if (is_first) {
                std::visit(NodePrinter{out, space}, elem.GetValue());
                is_first = false;
            } else {
                out << ",";
                std::visit(NodePrinter{out, space}, elem.GetValue());
            }
        }
        out << "]" << '\n';
    }
    
    void operator()(Dict dict) const {
        space.PrintIndent();
        out << "{" << '\n' << "  ";
        bool is_first = true;
        for (auto& elem : dict) {
            if (!is_first) {
                out << ",";
            }
            out << "\"" << elem.first << "\"" << ":";
            std::visit(NodePrinter{out, space.Indented()}, elem.second.GetValue());
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
        PrintString(val, out);
    }
};

void Print(const Document& doc, std::ostream& output);
    
}  // namespace json
