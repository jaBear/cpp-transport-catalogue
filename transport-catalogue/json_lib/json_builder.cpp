#include "json_builder.h"
 
namespace json {

Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
 
KeyContext Builder::BaseContext::Key(const std::string& key) {
    return builder_.Key(key);
}
    
Builder& Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(value);
}
 
DictionaryContext Builder::BaseContext::StartDict() {
    return DictionaryContext(builder_.StartDict());
}
    
Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}
 
ArrayContext Builder::BaseContext::StartArray() {
    return ArrayContext(builder_.StartArray());
}
    
Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

KeyContext::KeyContext(Builder& builder) : BaseContext(builder) {}
 
DictionaryContext  KeyContext::Value(Node::Value value) {
    return BaseContext::Value(std::move(value));
}

DictionaryContext::DictionaryContext(Builder& builder) : BaseContext(builder) {}

ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder) {}
 
ArrayContext ArrayContext::Value (Node::Value value) {
    return BaseContext::Value(move(value));
}
 
Node Builder::MakeNode(Node::Value value_) {
    Node node;
 
    if (std::holds_alternative<bool>(value_)) {
        node = Node(std::get<bool>(value_));
 
    } else if (std::holds_alternative<int>(value_)) {
        node = Node(std::get<int>(value_));
 
    } else if (std::holds_alternative<double>(value_)) {
        node = Node(std::get<double>(value_));
 
    } else if (std::holds_alternative<std::string>(value_)) {
        node = Node(std::move(std::get<std::string>(value_)));
 
    } else if (std::holds_alternative<Array>(value_)) {
        node = Node(std::move(std::get<Array>(value_)));
 
    } else if (std::holds_alternative<Dict>(value_)) {
        node = Node(std::move(std::get<Dict>(value_)));
 
    } else {
        node = Node();
    }
 
    return node;
}
 
void Builder::AddNode(Node node) {
    if (nodes_stack_.empty()) {
 
        if (!root_.IsNull()) {
            throw std::logic_error("root has been added");
        }
 
        root_ = node;
        return;
 
    } else {
 
        if (!nodes_stack_.back()->IsArray()
            && !nodes_stack_.back()->IsString()) {
 
            throw std::logic_error("unable to create node");
        }
 
        if (nodes_stack_.back()->IsArray()) {
            std::visit([this](auto&& arg){std::get<Array>(this->nodes_stack_.back()->GetValue()).emplace_back(arg);}, node.GetValue());
//            Array arr = nodes_stack_.back()->AsArray();
//            arr.emplace_back(node);
//            std::unique_ptr<Node> node = std::make_unique<Node>(arr);
//
//            nodes_stack_.pop_back();
//            nodes_stack_.push_back(std::move(node));
 
            return;
        }
 
        if (nodes_stack_.back()->IsString()) {
            std::string str = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
 
            if (nodes_stack_.back()->IsMap()) {
                std::visit([this, str](auto&& arg){std::get<Dict>(this->nodes_stack_.back()->GetValue())[str] = arg;}, node.GetValue());
//                Dict dictionary = nodes_stack_.back()->AsMap();
//                dictionary.emplace(std::move(str), node);
//
//                nodes_stack_.pop_back();
//                nodes_stack_.emplace_back(std::move(std::make_unique<Node>(dictionary)));
//                std::visit([this](auto&& arg){std::get<Array>(this->nodes_stack_.back()->GetValue()).emplace_back(arg);}, node.GetValue());
            }
 
            return;
        }
    }
}
 
KeyContext Builder::Key(const std::string& key_) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to create key");
    }
    auto key_ptr = std::make_unique<Node>(key_);
    
    if (nodes_stack_.back()->IsMap()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }
    return KeyContext(*this);
}
 
Builder& Builder::Value(Node::Value value_) {
    AddNode(MakeNode(value_));
    return *this;
}
 
DictionaryContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::make_unique<Node>(Dict()));
    return DictionaryContext(*this);
}
 
Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close as without opening");
    }
    Node node = *nodes_stack_.back();
 
    if (!node.IsMap()) {
        throw std::logic_error("object isn't dictionary");
    }
    nodes_stack_.pop_back();
    AddNode(node);
    return *this;
}
 
ArrayContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
    return ArrayContext(*this);
}
 
Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close without opening");
    }
    Node node = *nodes_stack_.back();
 
    if (!node.IsArray()) {
        throw std::logic_error("object isn't array");
    }
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}
 
Node Builder::Build() {
    if (root_.IsNull()) {
        throw std::logic_error("empty json");
    }
 
    if (!nodes_stack_.empty()) {
        throw std::logic_error("invalid json");
    }
    return root_;
}
 
}
