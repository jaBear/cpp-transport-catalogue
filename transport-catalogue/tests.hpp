#pragma once
#include <cassert>
#include "input_reader.hpp"
#include "transport_catalogue.hpp"

namespace test {

void AddOneStop() {
    InputReader input;
    TransportCatalogue test_catalogue;
    Stop test_stop;
    test_stop.name = "Tolstopaltsevo";
    test_stop.coordinates = {55.611087, 37.208290};
    input.ParseLine("Stop Tolstopaltsevo: 55.611087, 37.208290");
    input.ApplyCommands(test_catalogue);
    assert(test_catalogue.GetLastAddedStopName() == "Tolstopaltsevo");
    assert(test_catalogue.GetLastAddedStop() == test_stop);
}

void AddFiveStops() {
    InputReader input;
    TransportCatalogue test_catalogue;
    Stop test_stop;
    test_stop.name = "Biryusinka";
    test_stop.coordinates = {55.581065, 37.648390};
    input.ParseLine("Stop Tolstopaltsevo: 55.611087, 37.208290");
    input.ParseLine("Stop Marushkino: 55.595884, 37.209755");
    input.ParseLine("Stop Rasskazovka: 55.632761, 37.333324");
    input.ParseLine("Stop Biryulyovo Zapadnoye: 55.574371, 37.651700");
    input.ParseLine("Stop Biryusinka: 55.581065, 37.648390");
    input.ApplyCommands(test_catalogue);
    assert(test_catalogue.GetLastAddedStopName() == "Biryusinka");
    assert(test_catalogue.GetLastAddedStop() == test_stop);
    assert(test_catalogue.GetSizeOfStops() == 5);
    Stop test_stop2;
    test_stop2.name = "Rasskazovka";
    test_stop2.coordinates = {55.581065, 37.648390};
    test_catalogue.AddStop(std::move(test_stop2));
    assert(test_catalogue.GetSizeOfStops() == 5);
    assert(test_catalogue.GetLastAddedStop() == test_stop);
}

void AddBus() {
    InputReader input;
    TransportCatalogue test_catalogue;
    input.ParseLine("Stop Tolstopaltsevo: 55.611087, 37.208290");
    input.ParseLine("Stop Marushkino: 55.595884, 37.209755");
    input.ParseLine("Stop Rasskazovka: 55.632761, 37.333324");
    input.ParseLine("Stop Biryulyovo Zapadnoye: 55.574371, 37.651700");
    input.ParseLine("Stop Biryusinka: 55.581065, 37.648390");
    input.ParseLine("Bus 256: Biryulyovo Zapadnoye > Biryusinka > Rasskazovka > Biryusinka > Biryulyovo Zapadnoye");
    input.ApplyCommands(test_catalogue);
    assert(test_catalogue.GetSizeOfBuses() == 1);
    assert(test_catalogue.GetLastAddedBusName() == "256");
    test_catalogue.GetRouteInfo("256");
}
void Input() {
    AddOneStop();
    AddFiveStops();
    AddBus();
}
}
