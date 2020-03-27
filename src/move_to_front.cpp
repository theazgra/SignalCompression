#include "move_to_front.h"

azgra::ByteArray get_alphabet_from_text(const azgra::ByteArray &data)
{
    const auto uniqueChars = azgra::collection::distinct(data.cbegin(), data.cend());
    std::vector<azgra::byte> alphabet(uniqueChars.begin(), uniqueChars.end());
    std::sort(alphabet.begin(), alphabet.end());
    return alphabet;
}

inline void move_index_to_front(std::vector<azgra::byte> &alphabet, const std::size_t moveIndex)
{
    if (moveIndex == 0)
        return;

    for (std::size_t swapIndex = moveIndex; swapIndex > 0; --swapIndex)
    {
        std::swap(alphabet[swapIndex], alphabet[swapIndex - 1]);
    }
}


[[maybe_unused]] static void print_alphabet(const std::vector<azgra::byte> &alphabet)
{
    std::stringstream ss;
    for (unsigned char i : alphabet)
    {
        ss << (char) i;
    }
    puts(ss.str().c_str());
}

MTFResult encode_with_move_to_front(const azgra::ByteArray &data)
{
    const std::size_t dataSize = data.size();
    auto alphabet = get_alphabet_from_text(data);
    const std::size_t alphabetSize = alphabet.size();

    std::vector<std::size_t> indices(dataSize);

    for (std::size_t tIndex = 0; tIndex < dataSize; ++tIndex)
    {
        for (std::size_t alphabetIndex = 0; alphabetIndex < alphabetSize; ++alphabetIndex)
        {
            if (data[tIndex] == alphabet[alphabetIndex])
            {
                indices[tIndex] = alphabetIndex;

                // Move matched character to front
                move_index_to_front(alphabet, alphabetIndex);
                break;
            }
        }
    }

    const auto mtf_indicesEntropy = calculate_entropy(indices);
    fprintf(stdout, "Move-To-Front indices entropy: %.4f\n", mtf_indicesEntropy);

    auto rleEncodedIndices = rle_encode(indices);
    return MTFResult(std::move(alphabet), std::move(rleEncodedIndices), indices.size());
}

azgra::ByteArray decode_move_to_front(const MTFResult &mtf)
{
    auto alphabet = mtf.alphabet;
    std::sort(alphabet.begin(), alphabet.end());

    const auto indices = rle_decode(mtf.rleEncodedIndices, mtf.indicesCount);

    const std::size_t resultSize = mtf.indicesCount;
    azgra::ByteArray result(resultSize);

    std::size_t alphabetIndex;
    for (std::size_t i = 0; i < resultSize; ++i)
    {
        alphabetIndex = indices[i];
        result[i] = alphabet[alphabetIndex];
        move_index_to_front(alphabet, alphabetIndex);
    }

    return result;
}
