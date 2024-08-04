#pragma once
 
#include "json.h"
#include <stack>
#include <string>
#include <memory>
 

namespace json {

using namespace json;

class KeyContext;
class DictionaryContext;
class ArrayContext;
 
class Builder {
public:
    class BaseContext;
    
    KeyContext Key(const std::string& key_);
    DictionaryContext StartDict();
    ArrayContext StartArray();
    Builder& Value(Node::Value value);
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
 
private:
    void AddNode(Node node);
    Node MakeNode(Node::Value value_);
    
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
 
class Builder::BaseContext {
public:
    BaseContext(Builder& builder);
    KeyContext Key(const std::string& key);
    DictionaryContext StartDict();
    ArrayContext StartArray();
    Builder& Value(Node::Value value);
    Builder& EndDict();
    Builder& EndArray();
 
protected:
    Builder& builder_;
 
};
 
class KeyContext : public Builder::BaseContext {
public:
    KeyContext(Builder& builder);
    KeyContext Key(const std::string& key) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    DictionaryContext Value(Node::Value value);
};
 
class DictionaryContext : public Builder::BaseContext {
public:
    DictionaryContext(Builder& builder);
    DictionaryContext StartDict() = delete;
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
    Builder& Value(Node::Value value) = delete;
};
 
class ArrayContext : public Builder::BaseContext {
public:
    ArrayContext(Builder& builder);
    KeyContext Key(const std::string& key) = delete;
    Builder& EndDict() = delete;
    ArrayContext Value(Node::Value value);
};
 
}
