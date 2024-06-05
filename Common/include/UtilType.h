/**
 * @file UtilType.h
 * @author cooronx
 * @brief 用于和图形API交互的简易的矩阵，向量包装类
 * (起因是因为不知道为什么eigen的rowvector在传递的时候有问题)
 * @version 0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef UTILTYPE_H
#define UTILTYPE_H

#include <cstddef>
namespace MaxEngine
{
namespace Common
{
/**
 * @brief 矩阵包装类型(默认是列主序)
 *
 */
class MaxFloat4x4
{
private:
    float m_[4][4];

public:
    MaxFloat4x4() = default;
    MaxFloat4x4(const MaxFloat4x4 &) = default;
    MaxFloat4x4(MaxFloat4x4 &&) = default;
    MaxFloat4x4 &operator=(const MaxFloat4x4 &) = default;
    MaxFloat4x4 &operator=(MaxFloat4x4 &&) = default;
    MaxFloat4x4(float *raw_data)
    {
        m_[0][0] = raw_data[0];
        m_[0][1] = raw_data[1];
        m_[0][2] = raw_data[2];
        m_[0][3] = raw_data[3];

        m_[1][0] = raw_data[4];
        m_[1][1] = raw_data[5];
        m_[1][2] = raw_data[6];
        m_[1][3] = raw_data[7];

        m_[2][0] = raw_data[8];
        m_[2][1] = raw_data[9];
        m_[2][2] = raw_data[10];
        m_[2][3] = raw_data[11];

        m_[3][0] = raw_data[12];
        m_[3][1] = raw_data[13];
        m_[3][2] = raw_data[14];
        m_[3][3] = raw_data[15];
    }

    // Operators
    float operator()(size_t row, size_t col) const { return m_[row][col]; }
    float &operator()(size_t row, size_t col) { return m_[row][col]; }
};

/**
 * @brief 三维浮点行向量
 *
 */
class MaxFloat3
{
private:
    float x_;
    float y_;
    float z_;

public:
    MaxFloat3(float x, float y, float z) noexcept : x_(x), y_(y), z_(z){};
    MaxFloat3(const float *dt) noexcept : x_(dt[0]), y_(dt[1]), z_(dt[2]){};

    // getter and settter
    void setX(float nx) { x_ = nx; }
    void setY(float ny) { y_ = ny; }
    void setZ(float nz) { z_ = nz; }
    float getX() const { return x_; };
    float getY() const { return y_; };
    float getZ() const { return z_; };
};

/**
 * @brief 四维浮点行向量
 *
 */
class MaxFloat4
{
private:
    float x_;
    float y_;
    float z_;
    float w_;

public:
    MaxFloat4(float x, float y, float z, float w) noexcept : x_(x), y_(y), z_(z), w_(w){};
    MaxFloat4(const float *dt) noexcept : x_(dt[0]), y_(dt[1]), z_(dt[2]), w_(dt[3]){};

    // getter and settter
    void setX(float nx) { x_ = nx; }
    void setY(float ny) { y_ = ny; }
    void setZ(float nz) { z_ = nz; }
    void setW(float nw) { w_ = nw; }
    float getX() const { return x_; };
    float getY() const { return y_; };
    float getZ() const { return z_; };
    float getW() const { return w_; }
};
using MaxColor4f = MaxFloat4;

} // namespace Common

} // namespace MaxEngine

#endif