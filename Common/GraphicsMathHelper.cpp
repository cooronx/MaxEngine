#include "GraphicsMathHelper.h"

using namespace MaxEngine::Common;
Matrix4f GraphicsMathHelper::GetProjectionMatrix(float fov_y, float ratio, float n, float f)
{
    Matrix4f res{};
    res.setIdentity();
    fov_y = static_cast<float>(fov_y / 180.0 * MAXENGINE_PI);
    res << 1.0 / (ratio * std::tan(fov_y / 2)), 0, 0, 0, 0, 1.0 / (std::tan(fov_y / 2)), 0, 0, 0, 0,
        f / (f - n), 1, 0, 0, -n * f / (f - n), 0;
    return res;
}

Matrix4f GraphicsMathHelper::GetViewMatrix(Vector3f eye, Vector3f center, Vector3f up)
{
    Matrix4f res{};
    res.setIdentity();
    // 摄像机方向向量
    Vector3f w = center - eye;
    w.normalize();
    // 摄像机的右方向（对于D3D来说其实是左手系）
    Vector3f u = up.cross(w);
    u.normalize();
    // 摄像机的上方向
    Vector3f v = w.cross(u);
    v.normalize();
    res << u.x(), v.x(), w.x(), 0, u.y(), v.y(), w.y(), 0, u.z(), v.z(), w.z(), 0, -1 * eye.dot(u),
        -1 * eye.dot(v), -1 * eye.dot(w), 1;

    return res;
}