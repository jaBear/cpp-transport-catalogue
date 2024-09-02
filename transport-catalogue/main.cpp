#include <iostream>
#include <fstream>
#include <string>

#include "json_lib/json_reader.h"
//#include "log_duration.hpp"


using namespace std;

int main() {
//    LOG_DURATION("test");
    ifstream in("/Users/a1/Downloads/s12_final_opentest/s12_final_opentest_3.json");
    ofstream out("/Users/a1/Desktop/test.out");
//    ofstream out("/Users/a1/Desktop/srttest/out.txt");
    JsonReader request_handler;
    request_handler.Process(in, out);
}
