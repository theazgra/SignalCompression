#pragma once

#include <map>
#include <azgra/azgra.h>
#include <azgra/span.h>
#include "symbol_info.h"

double calculate_entropy(const std::map<azgra::byte, SymbolInfo> &symbolMap);

std::map<azgra::byte, SymbolInfo> get_symbols_info(const azgra::ByteSpan &data);