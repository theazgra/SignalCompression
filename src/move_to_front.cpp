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

std::vector<std::size_t> apply_move_to_front_coding(const azgra::ByteArray &data)
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
    return indices;
}
