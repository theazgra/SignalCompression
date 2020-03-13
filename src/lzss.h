#pragma once

#include "lz_tree.h"
#include "ring_buffer.h"
#include "sliding_window.h"
#include <random>
#include <azgra/io/stream/memory_bit_stream.h>

azgra::ByteArray lzss_encode(const azgra::ByteArray &data, const azgra::byte searchBufferBits);

void test_lzss();
void test();
void test2();
