#pragma once

#include "generic_huffman.h"
#include <azgra/azgra.h>
#include <iostream>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/collection/enumerable_functions.h>
#include <azgra/io/binary_file_functions.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <azgra/io/text_file_functions.h>
#include "entropy.h"

void huffman_encode(azgra::io::stream::OutMemoryBitStream &stream,
                    const huffman::HuffmanTree<azgra::StringView::value_type> &huffman,
                    const azgra::StringView textToEncode);

std::string huffman_decode(azgra::io::stream::InMemoryBitStream &stream);

//void test_huffmann(azgra::BasicStringView<char> inputFile)
//{
//    const auto text = azgra::io::read_text_file(inputFile);
//    const auto textView = azgra::StringView(text);
//    auto symbols = get_symbols_info(textView);
//    size_t totalSymbolCount = 0;
//
//    std::vector<std::pair<char, size_t>> symbols2;
//
//    for (const auto[symbol, info] : symbols)
//    {
//        totalSymbolCount += info.occurrenceCount;
//        symbols2.emplace_back(symbol, info.occurrenceCount);
//    }
//
//    const huffman::HuffmanTree huffman = huffman::build_huffman_tree(symbols2);
//
//    azgra::io::stream::OutMemoryBitStream outBitStream;
//    // We are passing symbols, so that we can write tree into the stream.
//    huffman_encode(outBitStream, huffman, textView);
//
//    const auto encodedBytes = outBitStream.get_flushed_buffer();
//    azgra::io::dump_bytes(encodedBytes, "huffman.data");
//
//    const auto originalByteCount = text.size();
//    const auto encodedByteCount = encodedBytes.size();
//    const float BPS = (static_cast<float>(encodedByteCount * 8)) / static_cast<float>(totalSymbolCount);
//    fprintf(stdout, "Original size: %lu; Encoded size: %lu\n", originalByteCount, encodedByteCount);
//
//    const auto inBuffer = azgra::io::stream::InBinaryFileStream("huffman.data").consume_whole_file();
//
//    azgra::io::stream::InMemoryBitStream decodeStream(&inBuffer);
//    std::string decoded = huffman_decode(decodeStream);
//
//
//    azgra::StringView decodedView(decoded);
//    azgra::io::write_text("decoded_text.txt", decodedView);
//    bool eq = textView == decodedView;
//
//    if (eq)
//    {
//        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Huffman encode/decode OK. BPS: %.4f %s\n\n",
//                               BPS,
//                               inputFile.data());
//    }
//    else
//    {
//        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Huffman encode/decode ERROR. %s\n\n", inputFile.data());
//
//    }
//}