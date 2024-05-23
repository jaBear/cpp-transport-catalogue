#include "json.hpp"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Error");
    }

    return Node(move(result));
}

std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (!input) {
        throw ParsingError("Error");
    }

    return Node(move(result));
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit or bool is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}



Node LoadNode(istream& input) {

    char c;
    input >> c;
//    if (c == '\t' || c == '\r' || c == '\n' || c == ' ') {
//        do {
//            input >> c;
//        } while (c == '\t' || c == '\r' || c == '\n' || c == ' ');
//    }
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        try {
            std::string result = LoadString(input);
            if (result == "true" || result == "false") {
                return (result == "true") ? Node(true) : Node(false);
            }
            return Node(result);
        } catch (const std::exception& except) {
            throw ParsingError("Error");
        }
        
    } else {
        input.putback(c);
        try {
            Number num = LoadNumber(input);
            std::string type;
            std::visit(ValChecker{type}, num);
            if (type == "int") {
//                input.putback(c);
                return Node((get<int>(num)));
            }
//            input.putback(c);
            return Node((get<double>(num)));
        } catch (const std::runtime_error& e) {
            string result;
            while (std::isalpha(input.peek())) {
                result.push_back(static_cast<char>(input.get()));
            }
            if (result == "true" || result == "false") {
                return (result == "true") ? Node(true) : Node(false);
            }
            if (result == "null") {
                return Node(nullptr);
            }
            
            throw ParsingError("Error");

        }
    }
}
}  // namespace

//Конструкторы Node
//Node::Node(nullptr_t) : Node() {}
//Node::Node(Array array) : val_(move(array)) {}
//Node::Node(Dict map) : val_(move(map)) {}
//Node::Node(bool val) : val_(val) {}
//Node::Node(int value) : val_(value) {}
//Node::Node(double value) : val_(value) {}
//Node::Node(string value) : val_(move(value)) {}

//Методы Node
const Node::Value& Node::GetValue() const {
    return *this;
}
const std::string Node::GetNodeObj() const {
    std::string result;
    std::visit(Node::NodeChecker{result}, GetValue());
    return result;
}

//Методы NodeChecker(Visitor для variant)
void Node::NodeChecker::operator()(std::nullptr_t) const {
    result = "nullptr";
}
void Node::NodeChecker::operator()(Array) const {
    result = "array";
}
void Node::NodeChecker::operator()(Dict) const {
    result = "map";
}
void Node::NodeChecker::operator()(bool) const {
    result = "bool";
}
void Node::NodeChecker::operator()(int) const {
    result = "int";
}
void Node::NodeChecker::operator()(double) const {
    result = "double";
}
void Node::NodeChecker::operator()(std::string) const {
    result = "string";
}

bool Node::IsNull() const {
    return std::holds_alternative<nullptr_t>(*this);
}
bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}
bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}
bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Value not array");
    }
    return get<Array>(*this);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Value not map");
    }
    return get<Dict>(*this);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Value not int");
    }
    return get<int>(*this);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Value not double");
    }
    if (IsInt()) {
        return static_cast<double>(get<int>(*this));
    }
    return get<double>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Value not bool");
    }
    return get<bool>(*this);
}



const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Value not string");
    }
    return get<string>(*this);
}


Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Node& Document::GetRoot() {
    return root_;
}

bool Document::operator ==(const Document& rhs) {
    return this->GetRoot() == rhs.GetRoot();
}
bool Document::operator!=(const Document& rhs) {
    return !(*this == rhs);
}


Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintString(std::string words, std::ostream& out) {
    out << "\""sv;
    for (char c : words) {
        switch (c) {
        case'\\':
            out << "\\\\"sv;
            break;

        case'"':
            out << "\\\""sv;
            break;

        case'\n':
            out << "\\n"sv;
            break;

        case'\r':
            out << "\\r"sv;
            break;

        case'\t':
            out << "\\t"sv;
            break;

        default:
            out << c;
            break;
        }
    }
    out << "\""sv;
}

using namespace std::literals;

void Print(const Document& doc, std::ostream& output) {
    std::visit(NodePrinter{output}, doc.GetRoot().GetValue());    
}

}  // namespace json
