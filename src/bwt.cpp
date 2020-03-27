#include "bwt.h"

static void print_matrix(const std::vector<azgra::ByteArray> &matrix)
{
    std::stringstream ss;
    const std::size_t dims = matrix.size();
    for (std::size_t row = 0; row < dims; ++row)
    {
        for (std::size_t col = 0; col < dims; ++col)
        {
            ss << static_cast<char>(matrix[row][col]);
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

azgra::ByteArray apply_burrows_wheeler_transform(const azgra::ByteSpan &data)
{

    const std::size_t dataSize = data.size();
    azgra::ByteArray L(dataSize);
    std::size_t I;
    {
        std::vector<azgra::ByteArray> mat(dataSize);

        azgra::ByteArray firstRow(dataSize);
        std::memcpy(firstRow.data(), data.data(), dataSize);
        // Copy the first row, so we can find I value later.
        mat[0] = firstRow;

        for (std::size_t row = 1; row < dataSize; ++row)
        {
            azgra::ByteArray rowData(dataSize);
            std::rotate_copy(mat[0].begin(), (mat[0].begin() + row), mat[0].end(), rowData.begin());
            mat[row] = std::move(rowData);
        }
        std::sort(mat.begin(), mat.end(), [](const azgra::ByteArray &a, const azgra::ByteArray &b)
        {
            return (std::memcmp(a.data(), b.data(), a.size()) < 0);
        });

        auto originalDataIt = std::find(mat.begin(), mat.end(), firstRow);
        assert(originalDataIt != mat.end());
        I = (originalDataIt - mat.begin());

        print_matrix(mat);

        for (std::size_t row = 0; row < dataSize; ++row)
        {
            L[row] = mat[row][dataSize - 1];
        }
    }

    // The L and I  represents the compressed textData.
    puts("L");
    print_vector(L);
    fprintf(stdout, "I=%lu\n", I);

    puts("F");
    azgra::ByteArray F(L.begin(), L.end());
    std::sort(F.begin(), F.end());
    print_vector(F);

    std::vector<int> T(dataSize);
    for (std::size_t i = 0; i < dataSize; ++i)
    {
        for (std::size_t searchIndex = 0; searchIndex < dataSize; ++searchIndex)
        {
            const auto searchEnd = T.begin() + i;
            if ((L[i] == F[searchIndex]) && (std::find(T.begin(), searchEnd, searchIndex) == searchEnd))
            {
                T[i] = searchIndex;
                break;
            }
        }
//        T[i] = (std::find(F.begin(), F.end(), L[i]) - F.begin());
    }
    puts("T");
    print_vector<int>(T);

    azgra::ByteArray reconstructedS(dataSize);

    std::function<std::size_t(const std::size_t, const std::size_t)> TFunc =
            [&](const std::size_t i, const std::size_t j) -> std::size_t
            {
                if (i == 0)
                {
                    return j;
                }
                return T[TFunc(i - 1, j)];
            };

    for (std::size_t i = 0; i < dataSize; ++i)
    {
        reconstructedS[(dataSize - 1) - i] = L[TFunc(i, I)];
    }
    puts("reconstructedS");
    print_vector(reconstructedS);
    /*
     *  RS[n − 1 − i] ← L[Ti[I]], for i = 0, 1, . . . , n − 1,
        where T0[j] = j, and Ti+1[j] = T[Ti[j]].
     * */

    return L;
}
