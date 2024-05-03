#pragma once

#include <iosfwd>
#include <iostream>
#include <string_view>

#include "transport_catalogue.hpp"

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

void CommandDescriptionTest();
