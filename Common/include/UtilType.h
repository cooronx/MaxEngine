#ifndef UTILTYPE_H
#define UTILTYPE_H

#include <cstddef>
namespace MaxEngine
{
namespace Common
{
/**
 * @brief 从一个列主序矩阵转换普通的二维数组
 *
 */
class Float4x4
{
    private:
    float m[4][4];

    public:
    Float4x4() = default;
    Float4x4(float *raw_data)
    {
        m[0][0] = raw_data[0];
        m[0][1] = raw_data[1];
        m[0][2] = raw_data[2];
        m[0][3] = raw_data[3];

        m[1][0] = raw_data[4];
        m[1][1] = raw_data[5];
        m[1][2] = raw_data[6];
        m[1][3] = raw_data[7];

        m[2][0] = raw_data[8];
        m[2][1] = raw_data[9];
        m[2][2] = raw_data[10];
        m[2][3] = raw_data[11];

        m[3][0] = raw_data[12];
        m[3][1] = raw_data[13];
        m[3][2] = raw_data[14];
        m[3][3] = raw_data[15];
    }

    // Operators
    float operator()(size_t row, size_t col) const
    {
        return m[row][col];
    }
    float &operator()(size_t row, size_t col)
    {
        return m[row][col];
    }
};

} // namespace Common

} // namespace MaxEngine

#endif