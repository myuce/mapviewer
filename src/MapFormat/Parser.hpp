#pragma once

#include "Lexer.hpp"
#include "map.hpp"

// Top-level entry point: fills in outMap and returns true on success.
bool parseMap(Lexer &lx, Map *map);

