#ifndef GRAPHICSMATHHELPER_H
#define GRAPHICSMATHHELPER_H

#include <Eigen/Core>
#include <Eigen/Dense>

using Eigen::Matrix4f;
using Eigen::Vector3f;

namespace MaxEngine
{
namespace Common
{

class GraphicsMathHelper
{
public:
    GraphicsMathHelper() = delete;
    GraphicsMathHelper(const GraphicsMathHelper &) = delete;
    GraphicsMathHelper(GraphicsMathHelper &&) = delete;

    /**
     * @brief 左手观察矩阵
     *
     * @param eye 相机的世界坐标
     * @param center 看向中心点的世界坐标
     * @param up 世界的上方向
     * @return Matrix4f 观察矩阵（左乘形式，也就是行向量乘以）
     */
    static Matrix4f GetViewMatrix(Vector3f eye, Vector3f center, Vector3f up);
    /**
     * @brief 左手观察矩阵
     *
     * @param fov_y 垂直fov(角度)
     * @param ratio 宽高比
     * @param n 近平面
     * @param f 远平面
     * @return Matrix4f 观察矩阵（左乘形式，也就是行向量乘以,这样符合D3D的标准）
     */
    static Matrix4f GetProjectionMatrix(float fov_y, float ratio, float n, float f);
};

} // namespace Common

} // namespace MaxEngine

#endif