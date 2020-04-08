#include <azgra/string/smart_string_view.h>
#include "lzw.h"


static std::vector<azgra::StringView> get_words(const azgra::StringView &text)
{
    const azgra::string::SmartStringView ssw(text);
    // We don't need SmartStringViews.
    const auto smartStringViewWords = ssw.split_to_words();
    std::vector<azgra::StringView> words(smartStringViewWords.size());
    for (std::size_t i = 0; i < smartStringViewWords.size(); ++i)
    {
        words[i] = smartStringViewWords[i].string_view();
    }
    return words;
}

robin_hood::unordered_set<azgra::StringView> get_lzw_dictionary(const azgra::StringView &text)
{
    const auto words = get_words(text);
    robin_hood::unordered_set<azgra::StringView> dictionary;

    // Initialize dictionary with all unique words.
    for (const auto word : words)
    {
        dictionary.insert(word);
    }

    azgra::StringView IWord = words[0];
    // We skip the first word, which must be in the dictionary.
    bool lastWasFound = false;
    for (std::size_t i = 1; i < words.size(); ++i)
    {
        lastWasFound = false;
        // add word[i] to IWord `buffer` and check if new IWord is in dictionary.
        IWord = azgra::StringView(IWord.data(), IWord.size() + words[i].size() + 1);
        if (dictionary.contains(IWord))
        {
            lastWasFound = true;
            continue;
        }

        // Add IWord to dictionary.
        dictionary.insert(IWord);
        IWord = words[i];
    }

    if (!lastWasFound)
    { dictionary.insert(words[words.size() - 1]); }

    return dictionary;
}

azgra::f64 calculate_fcd(const robin_hood::unordered_set<azgra::StringView> &xDict,
                         const robin_hood::unordered_set<azgra::StringView> &yDict)
{
    const auto xSize = static_cast<azgra::f64> (xDict.size());

    std::size_t interSecSize = 0;
    for (const auto xVal : xDict)
    {
        if (yDict.contains(xVal))
        {
            ++interSecSize;
        }
    }

    const auto intersectionSize = static_cast<azgra::f64>(interSecSize);

    const azgra::f64 FCD = (xSize - intersectionSize) / xSize;
    return FCD;
}



















