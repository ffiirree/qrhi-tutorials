#ifndef RHI_ANIMATION_H
#define RHI_ANIMATION_H

#include "bone.h"
#include "model.h"
#include "utils.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <QMatrix4x4>

struct AssimpNodeData
{
    QMatrix4x4                  transform{};
    std::string                 name{};
    std::vector<AssimpNodeData> children{};
};

class Animation
{
public:
    Animation() = default;

    Animation(const std::string& path, Model *model)
    {
        Assimp::Importer importer{};
        const auto       scene = importer.ReadFile(path, aiProcess_Triangulate);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            qDebug() << importer.GetErrorString();
            return;
        }

        //
        const auto animation  = scene->mAnimations[0];
        auto&      bone_infos = model->bone_infos();
        duration_             = animation->mDuration;
        ticks_                = animation->mTicksPerSecond;

        for (unsigned i = 0; i < animation->mNumChannels; ++i) {
            const auto        channel = animation->mChannels[i];
            const std::string name    = channel->mNodeName.data;

            if (!bone_infos.contains(name)) {
                bone_infos[name] = { name, static_cast<int>(bone_infos.size()) };
            }
            bones_.emplace_back(name, bone_infos[name].id, channel);
        }

        bone_infos_ = bone_infos;

        //
        read_heirarchy_data(root_, scene->mRootNode);
    }

    Bone *find(const std::string& name)
    {
        for (auto& bone : bones_) {
            if (bone.name() == name) {
                return std::addressof(bone);
            }
        }
        return nullptr;
    }

    [[nodiscard]] double                   ticks() const { return ticks_; }
    [[nodiscard]] double                   duration() const { return duration_; }
    const AssimpNodeData&                  root_node() { return root_; }
    const std::map<std::string, BoneInfo>& bone_infos() { return bone_infos_; }

private:
    void read_heirarchy_data(AssimpNodeData& dest, const aiNode *src)
    {
        dest.name      = src->mName.data;
        dest.transform = utils::to_qmatrix4x4(src->mTransformation);

        for (unsigned i = 0; i < src->mNumChildren; ++i) {
            AssimpNodeData node{};
            read_heirarchy_data(node, src->mChildren[i]);
            dest.children.push_back(node);
        }
    }

private:
    double                          duration_{};
    double                          ticks_{};
    std::vector<Bone>               bones_{};
    AssimpNodeData                  root_{};
    std::map<std::string, BoneInfo> bone_infos_{};
};

#endif // RHI_ANIMATION_H
