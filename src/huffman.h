#pragma once

#include "generic_huffman.h"
#include <azgra/azgra.h>
#include <iostream>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/collection/enumerable_functions.h>
#include "symbol_info.h"

std::map<char, SymbolInfo> get_string_symbols_info(const azgra::StringView &string);

void huffman_encode(azgra::io::stream::OutMemoryBitStream &stream,
                    const huffman::HuffmanTree<azgra::StringView::value_type> &huffman,
                    const azgra::StringView textToEncode);

std::string huffman_decode(azgra::io::stream::InMemoryBitStream &stream);