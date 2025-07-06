#ifndef RHI_BONE_H
#define RHI_BONE_H

#include <assimp/scene.h>
#include <QMatrix4x4>
#include <utility>

struct KeyPosition
{
    QVector3D position{};
    double    timestamp{};
};

struct KeyRotation
{
    QQuaternion orientation{};
    double      timestamp{};
};

struct KeyScale
{
    QVector3D scale{};
    double    timestamp;
};

class Bone
{
public:
    Bone(std::string name, const int id, const aiNodeAnim *channel) : name_(std::move(name)), id_(id)
    {
        for (unsigned i = 0; i < channel->mNumPositionKeys; ++i) {
            const auto position = channel->mPositionKeys[i].mValue;

            positions_.push_back({
                .position  = { position.x, position.y, position.z },
                .timestamp = channel->mPositionKeys[i].mTime,
            });
        }

        for (unsigned i = 0; i < channel->mNumRotationKeys; ++i) {
            const auto orientation = channel->mRotationKeys[i].mValue;

            rotations_.push_back({
                .orientation = { orientation.w, orientation.x, orientation.y, orientation.z },
                .timestamp   = channel->mRotationKeys[i].mTime,
            });
        }

        for (unsigned i = 0; i < channel->mNumScalingKeys; ++i) {
            const auto scale = channel->mScalingKeys[i].mValue;

            scales_.push_back({
                .scale     = { scale.x, scale.y, scale.z },
                .timestamp = channel->mScalingKeys[i].mTime,
            });
        }
    }

    void update(const double time)
    {
        const auto translation = interpolate_position(time);
        const auto rotation    = interpolate_rotation(time);
        const auto scale       = interpolate_scale(time);
        transform_             = translation * rotation * scale;
    }

    int get_position_index(const double time)
    {
        for (int i = 0; i < positions_.size() - 1; ++i) {
            if (time < positions_[i + 1].timestamp) return i;
        }
        assert(0);
        return 0;
    }

    int get_rotation_index(const double time)
    {
        for (int i = 0; i < rotations_.size() - 1; ++i) {
            if (time < rotations_[i + 1].timestamp) return i;
        }
        assert(0);
        return 0;
    }

    int get_scale_index(const double time)
    {
        for (int i = 0; i < scales_.size() - 1; ++i) {
            if (time < scales_[i + 1].timestamp) return i;
        }
        assert(0);
        return 0;
    }

    [[nodiscard]] QMatrix4x4  local_transform() const { return transform_; }
    [[nodiscard]] std::string name() const { return name_; }
    [[nodiscard]] int         id() const { return id_; }

private:
    static double get_scale_factor(const double last_ts, const double next_ts, const double time)
    {
        return (time - last_ts) / (next_ts - last_ts);
    }

    QMatrix4x4 interpolate_position(const double time)
    {
        QMatrix4x4 position{};
        if (positions_.size() == 1) {
            position.translate(positions_[0].position);
            return position;
        }

        const auto idx = get_position_index(time);
        const auto factor =
            get_scale_factor(positions_[idx].timestamp, positions_[idx + 1].timestamp, time);
        position.translate(positions_[idx].position * factor +
                           positions_[idx + 1].position * (1.0 - factor));
        return position;
    }

    QMatrix4x4 interpolate_rotation(const double time)
    {
        QMatrix4x4 rm{};
        if (rotations_.size() == 1) {
            rm.rotate(rotations_[0].orientation.normalized());
            return rm;
        }

        const auto idx = get_rotation_index(time);
        const auto factor =
            get_scale_factor(rotations_[idx].timestamp, rotations_[idx + 1].timestamp, time);
        const auto final_rotation =
            QQuaternion::slerp(rotations_[idx].orientation, rotations_[idx + 1].orientation, factor);
        rm.rotate(final_rotation.normalized());
        return rm;
    }

    QMatrix4x4 interpolate_scale(const double time)
    {
        QMatrix4x4 scale{};
        if (scales_.size() == 1) {
            scale.scale(scales_[0].scale);
            return scale;
        }

        const auto idx    = get_scale_index(time);
        const auto factor = get_scale_factor(scales_[idx].timestamp, scales_[idx + 1].timestamp, time);
        scale.scale(scales_[idx].scale * factor + scales_[idx + 1].scale * (1.0 - factor));

        return scale;
    }

private:
    std::string name_{};
    int         id_;

    QMatrix4x4 transform_{};

    std::vector<KeyPosition> positions_{};
    std::vector<KeyRotation> rotations_{};
    std::vector<KeyScale>    scales_{};
};

#endif // RHI_BONE_H
