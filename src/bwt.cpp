#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"

#include "bwt.h"

[[maybe_unused]] static void print_matrix(const std::vector<CyclicSpan<azgra::byte>> &permutations)
{
    std::stringstream ss;
    const std::size_t dims = permutations.size();
    for (std::size_t row = 0; row < dims; ++row)
    {
        for (std::size_t col = 0; col < dims; ++col)
        {
            ss << static_cast<char>(permutations[row][col]);
        }
        ss << '\n';
    }
    puts(ss.str().c_str());
}

template<typename CastType=char>
static void print_vector(const std::vector<CastType> &data)
{
    std::stringstream ss;
    const std::size_t dims = data.size();
    for (std::size_t i = 0; i < dims; ++i)
    {
        ss << static_cast<CastType>(data[i]);
    }
    puts(ss.str().c_str());
}

BWTResult apply_burrows_wheeler_transform(const azgra::ByteSpan &S)
{
    const std::size_t dataSize = S.size();
    azgra::ByteArray L(dataSize);
    std::size_t I;
    {
        std::vector<CyclicSpan<azgra::byte>> permutations;
        permutations.reserve(dataSize);
        for (std::size_t row = 0; row < dataSize; ++row)
        {
            permutations.emplace_back(S.data(), dataSize, row);
        }
        std::sort(permutations.begin(), permutations.end());

        // NOTE(Moravec):   We don't have to create T array, because we will
        //                  construct L array from permutation spans.
        //                  So here we just find the I value.
        I = (std::find_if(permutations.begin(), permutations.end(), [](const CyclicSpan<azgra::byte> &permutation)
        {
            return (permutation.offset() == 0lu);
        }) - permutations.begin());
        for (std::size_t i = 0; i < dataSize; ++i)
        {
            // NOTE:    Book says to use the previous permutations first element, but
            //          we can do better with our cyclic span.
            L[i] = permutations[i][dataSize - 1];
        }
    }

    const double L_Entropy = calculate_entropy(L);
    fprintf(stdout, "BWT's L entropy = %.4f\n", L_Entropy);
    return BWTResult(encode_with_move_to_front(L), I);
}

azgra::ByteArray decode_burrows_wheeler_transform(const azgra::ByteArray &L, const std::size_t I)
{

    const std::size_t dataSize = L.size();

    std::vector<std::size_t> T(dataSize);
    {
        std::vector<bool> indexUsed(dataSize);
        azgra::ByteArray F(L.begin(), L.end());
        std::sort(F.begin(), F.end());
        puts("Constructing T");
        // Construct array T.

//#pragma omp parallel for
        for (std::size_t i = 0; i < dataSize; ++i)
        {
            for (std::size_t searchIndex = 0; searchIndex < dataSize; ++searchIndex)
            {
                // Speedup the duplicate ?
                if ((L[i] == F[searchIndex]) && !indexUsed[searchIndex])
                {
                    T[i] = searchIndex;
                    indexUsed[searchIndex] = true;
                    break;
                }
            }
            if ((i % 1000) == 0)
            {
                const double percFinished = (static_cast<double>(i) / static_cast<double>(dataSize)) * 100.0;
                fprintf(stdout, "T construction = %.4f %%\n", percFinished);
            }
        }
        // F is no longer needed here.
        puts("Created T");

    }
    azgra::ByteArray reconstructedS(dataSize);
    std::function<std::size_t(const std::size_t, const std::size_t)> TFunc =
            [&](const std::size_t i, const std::size_t j) -> std::size_t
            {
                if (i == 0)
                {
                    return j;
                }
                std::size_t current = j;
                for (int k = 0; k < i; ++k)
                {
                    current = T[current];
                }
                return current;
//                return T[TFunc(i - 1, j)];
            };

#pragma omp parallel for
    for (std::size_t i = 0; i < dataSize; ++i)
    {
        reconstructedS[(dataSize - 1) - i] = L[TFunc(i, I)];
    }
    return reconstructedS;
}

azgra::ByteArray encode_with_bwt_mtf_rle(const azgra::ByteSpan &dataSpan)
{
    BWTResult bwtResult = apply_burrows_wheeler_transform(dataSpan);

    const auto IBits = azgra::io::stream::bits_required(bwtResult.I);
    // If we align bits to 8 we get lesser entropy
    const auto alphabetIndexBits = azgra::io::stream::bits_required(bwtResult.mtf.alphabet.size());
    azgra::io::stream::OutMemoryBitStream bitStream;

    // Write bits for I value
    bitStream.write_value(static_cast<azgra::byte>(IBits));
    // Write I
    bitStream.write_value(bwtResult.I, IBits);

    // Write alphabet size
    bitStream.write_value(bwtResult.mtf.alphabet.size());
    // Write alphabet
    for (const azgra::byte alphabetByte : bwtResult.mtf.alphabet)
    {
        bitStream.write_value(alphabetByte);
    }

    // Write indices count
    bitStream.write_value(bwtResult.mtf.indices.size());

    // Write bits for alphabet indices.
    bitStream.write_value(static_cast<azgra::byte>(alphabetIndexBits));
    // Write indices
    for (const azgra::byte alphabetIndex : bwtResult.mtf.indices)
    {
        bitStream.write_value(alphabetIndex, alphabetIndexBits);
    }

    return bitStream.get_flushed_buffer();
}

azgra::ByteArray decode_bwt_mtf_rle(const azgra::ByteArray &encodedBytes)
{
    azgra::io::stream::InMemoryBitStream bitStream(&encodedBytes);

    const auto bitsForIValue = bitStream.read_value<azgra::byte>();
    const auto I = bitStream.read_value<std::size_t>(bitsForIValue);

    const auto alphabetSize = bitStream.read_value<std::size_t>();
    azgra::ByteArray alphabet(alphabetSize);
    for (std::size_t i = 0; i < alphabetSize; ++i)
    {
        alphabet[i] = bitStream.read_value<azgra::byte>();
    }

    const auto indicesSize = bitStream.read_value<std::size_t>();

    const std::size_t bitsPerIndex = bitStream.read_value<azgra::byte>();

    std::vector<std::size_t> indices(indicesSize);
    for (std::size_t i = 0; i < indicesSize; ++i)
    {
        indices[i] = bitStream.read_value<std::size_t>(bitsPerIndex);
    }
    MTFResult mtf(std::move(alphabet), std::move(indices));
    const auto decodedL = decode_move_to_front(mtf);

    auto decodedData = decode_burrows_wheeler_transform(decodedL, I);
    return decodedData;
}


#pragma clang diagnostic pop