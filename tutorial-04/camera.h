#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix4x4>

class Camera
{
public:
    Camera(const QVector3D& position, const QVector3D& center, const QVector3D& up)
        : position_(position), center_(center), up_(up)
    {}

    QMatrix4x4 view() { return lookAt(position_, center_, up_); }

    [[nodiscard]] QVector3D position() const { return position_; }

    static QMatrix4x4 lookAt(const QVector3D& position, const QVector3D& center,
                             const QVector3D& up = { 0.0f, 1.0f, 0.0f })
    {
        auto z = (position - center).normalized();
        auto x = QVector3D::normal(up, z);
        auto y = QVector3D::normal(z, x);

        return { x.x(), x.y(), x.z(), -position.x(), y.x(), y.y(), y.z(), -position.y(),
                 z.x(), z.y(), z.z(), -position.z(), 0.0f,  0.0f,  0.0f,  1.0f };
    }

private:
    QVector3D position_;
    QVector3D center_;
    QVector3D up_;
};

#endif //! CAMERA_H