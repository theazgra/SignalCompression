#pragma once
#include <azgra/azgra.h>

struct CompressionResult
{
    std::size_t originalSize{0};
    std::size_t compressedSize{0};
    double compressionRatio{1.0};
    double compressionTimeMS{0.0};
    double speed{0.0};
};