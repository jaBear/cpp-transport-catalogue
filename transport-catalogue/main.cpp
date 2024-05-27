#include <iostream>
#include <fstream>
#include <string>

#include "json_lib/json_reader.hpp"


using namespace std;

int main() {
    JsonReader request_handler;
    request_handler.Process(cin, cout);
}
