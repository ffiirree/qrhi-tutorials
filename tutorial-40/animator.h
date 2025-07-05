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
        final_bone_matrices_.resize(500);
    }

    void update(double dt)
    {
        delta_time_ = dt;
        if (animation_) {
            time_ += animation_->ticks() * dt;
            time_ = std::fmod(time_, animation_->duration());

            calculate_bone_transform(&animation_->root_node(), QMatrix4x4{});
        }
    }

    void play(Animation *animation)
    {
        animation_ = animation;
        time_      = 0.0;
    }

    void calculate_bone_transform(const AssimpNodeData *node, QMatrix4x4 transform)
    {
        const auto bone           = animation_->find(node->name);
        auto       node_transform = node->transform;

        if (bone) {
            bone->update(time_);
            node_transform = bone->local_transform();
        }

        auto global_transform = transform * node_transform;

        auto bone_infos = animation_->bone_infos();
        if (bone_infos.contains(node->name)) {
            int  idx                  = bone_infos[node->name].id;
            auto offset               = bone_infos[node->name].offset;
            final_bone_matrices_[idx] = global_transform * offset;
        }

        for (const auto& i : node->children) {
            calculate_bone_transform(&i, global_transform);
        }
    }

    [[nodiscard]] auto final_bone_matrices() const { return final_bone_matrices_; }

private:
    double                  time_{};
    double                  delta_time_{};
    std::vector<QMatrix4x4> final_bone_matrices_{};
    Animation              *animation_{};
};

#endif // RHI_ANIMATOR_H
