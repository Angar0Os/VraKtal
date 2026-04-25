#ifndef VRAKTAL_LOADERS_MESHLOADER_H
#define VRAKTAL_LOADERS_MESHLOADER_H
#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <atomic>

#include <graphics/resources/object/mesh.h>
#include <core/gpu/buffer.h>

#include "./loaderBase.h"

namespace loaders
{
    using JobID = uint64_t;

    enum class EJobStatus
    {
        Pending,
        Running,
        Done,
        Failed
    };

    struct Job
    {
        JobID                                           id = 0;
        std::string                                     name;
        std::function<void()>                           task;
        std::function<void()>                           onComplete;
        std::vector<JobID>                              dependencies;
        std::atomic<EJobStatus>                         status{ EJobStatus::Pending };

        Job() = default;
        Job(const Job&) = delete;
        Job& operator=(const Job&) = delete;

        Job(Job&& other) noexcept
            : id(other.id),
              name(std::move(other.name)),
              task(std::move(other.task)),
              onComplete(std::move(other.onComplete)),
              dependencies(std::move(other.dependencies)),
              status(other.status.load())
        {
        }

        Job& operator=(Job&& other) noexcept
        {
            if (this != &other)
            {
                id = other.id;
                name = std::move(other.name);
                task = std::move(other.task);
                onComplete = std::move(other.onComplete);
                dependencies = std::move(other.dependencies);
                status.store(other.status.load());
            }
            return *this;
        }
    };

    class MeshLoader :public LoaderBase
    {
    private:
        core::gpu::Device* m_device;
        std::vector<Job>            m_jobQueue;
        JobID                       m_nextJobID = 1;

        std::shared_ptr<graphics::resources::Mesh> LoadGLTF(const std::string& filepath);
        std::shared_ptr<graphics::resources::Mesh> LoadOBJ(const std::string& filepath);
        void CreateBuffersForMesh(graphics::resources::Mesh* mesh);
        void CreateBLASForMesh(graphics::resources::Mesh* mesh);

        bool AreDependenciesDone(const Job& job) const;
        JobID PushJob(std::string name,
            std::function<void()> task,
            std::vector<JobID> deps = {},
            std::function<void()> onComplete = nullptr);

    public:
        explicit MeshLoader(core::gpu::Device* device);

        JobID LoadMesh(const std::string& filepath, std::function<void(std::shared_ptr<graphics::resources::Mesh>)> onComplete = nullptr);

        JobID CreatePlane(float width, float height, int subdivisionsX = 1, int subdivisionsZ = 1,
                            std::function<void(std::shared_ptr<graphics::resources::Mesh>)> onComplete = nullptr);

        void ProcessJobs();
        void PurgeFinishedJobs();

        std::shared_ptr<void> Load(const std::string& path);
    };
}

#endif