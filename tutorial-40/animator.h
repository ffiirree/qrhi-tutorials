#ifndef RHI_ANIMATOR_H
#define RHI_ANIMATOR_H

#include "animation.h"

class Animator
{
public:
    Animator(Animation *animation)
    {
        time_      = 0.0;
        animation_ = animation;
        bone_matrices_.resize(500);
    }

    void update(const double dt)
    {
        delta_time_ = dt;
        if (animation_) {
            time_ += animation_->ticks() * dt;
            time_ = std::fmod(time_, animation_->duration());

            calculate_bone_transform(&animation_->root_node(), QMatrix4x4{});
        }
    }

    void calculate_bone_transform(const AssimpNodeData *node, const QMatrix4x4& accumulated_transform)
    {
        const auto bone           = animation_->find(node->name);
        auto       node_transform = node->transform;

        if (bone) {
            bone->update(time_);
            node_transform = bone->local_transform();
        }

        const auto transform = accumulated_transform * node_transform;

        if (auto bone_infos = animation_->bone_infos(); bone_infos.contains(node->name)) {
            const int idx       = bone_infos[node->name].id;
            bone_matrices_[idx] = transform * bone_infos[node->name].offset;
        }

        for (const auto& i : node->children) {
            calculate_bone_transform(&i, transform);
        }
    }

    [[nodiscard]] auto bone_matrices() const { return bone_matrices_; }

private:
    double                  time_{};
    double                  delta_time_{};
    std::vector<QMatrix4x4> bone_matrices_{};
    Animation              *animation_{};
};

#endif // RHI_ANIMATOR_H
