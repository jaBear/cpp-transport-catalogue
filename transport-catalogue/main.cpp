#include <iostream>
#include <fstream>
#include <string>

#include "json_lib/json_reader.hpp"


using namespace std;

int main() {
    
    ifstream infile;
    infile.open("/Users/a1/Desktop/C++/firstApp/secpap/teststs/route_info/route_info/primer.json", ios::in);
    
    ofstream ofile("/Users/a1/Desktop/C++/firstApp/secpap/teststs/route_info/route_info/output.json");
    
    JsonReader request_handler;
    request_handler.Process(infile, ofile);
    infile.close();

}
