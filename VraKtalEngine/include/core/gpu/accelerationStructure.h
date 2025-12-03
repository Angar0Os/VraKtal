#ifndef VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
#define VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
#pragma once

#include <memory>
#include <vector>
#include <cstdint>

namespace core::gpu
{
    class AccelerationStructure;
    class Buffer;
    class Device;

    enum class EAccelerationStructureType
    {
        BottomLevel, // (BLAS)
        TopLevel     // (TLAS)
    };

    struct SAccelerationStructureGeometry
    {
        Buffer* vertexBuffer = nullptr;
        uint32_t vertexCount = 0;
        uint32_t vertexStride = 0;

        Buffer* indexBuffer = nullptr;
        uint32_t indexCount = 0;
        uint32_t triangleCount = 0;

        Buffer* transformBuffer = nullptr;
        bool opaque = true;
    };

    struct SAccelerationStructureInstance
    {
        float transform[12];
        uint32_t instanceCustomIndex = 0;
        uint32_t mask = 0xFF;
        uint32_t instanceShaderBindingTableRecordOffset = 0;
        AccelerationStructure* blas = nullptr;
    };

    struct SAccelerationStructureCreateInfo
    {
        EAccelerationStructureType type;

        std::vector<SAccelerationStructureGeometry> geometries;
        std::vector<SAccelerationStructureInstance> instances;

        bool allowUpdate = false;
        bool preferFastTrace = true;
    };

    class AccelerationStructure
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        AccelerationStructure(const core::gpu::Device* _device,
                              const SAccelerationStructureCreateInfo& _info);
        ~AccelerationStructure();

        uint64_t GetDeviceAddress() const;

        void Build(void* commandBuffer);

        Impl& GetImpl() const;
    };
}

#endif //VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
