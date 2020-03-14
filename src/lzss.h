#pragma once

#include "lz_tree.h"
#include "ring_buffer.h"
#include "sliding_window.h"
#include <random>
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/io/stream/in_binary_file_stream.h>

constexpr uint8_t RAW_BYTE_FLAG = false;
constexpr uint8_t PAIR_FLAG = true;

azgra::ByteArray lzss_encode(const azgra::ByteArray &data,
                             const azgra::byte windowBits,
                             const azgra::byte searchBufferBits);

void test_lzss(const char* inputFile);

void test();

void test2();
