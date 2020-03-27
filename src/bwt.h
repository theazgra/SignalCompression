#pragma once

#include "move_to_front.h"
#include "entropy.h"
#include "cyclic_span.h"
#include <azgra/io/stream/memory_bit_stream.h>
#include <azgra/io/binary_file_functions.h>

struct BWTResult
{
    MTFResult mtf;
    std::size_t I;

    BWTResult(const BWTResult &) = delete;

    explicit BWTResult(MTFResult &&mtfResult, const std::size_t I_)
    {
        mtf = mtfResult;
        I = I_;
    }
};


BWTResult apply_burrows_wheeler_transform(const azgra::ByteSpan &S);

azgra::ByteArray decode_burrows_wheeler_transform(const azgra::ByteArray &L, const std::size_t I);

azgra::ByteArray encode_with_bwt_mtf_rle(const azgra::ByteSpan &dataSpan);

azgra::ByteArray decode_bwt_mtf_rle(const azgra::ByteArray &encodedBytes);