#ifndef VRAKTAL_CORE_GPU_VULKAN_ACCELERATION_STRUCTURE_H
#define VRAKTAL_CORE_GPU_VULKAN_ACCELERATION_STRUCTURE_H
#pragma once

#include <core/gpu/accelerationStructure.h>
#include <vulkan/vulkan_raii.hpp>

#include <memory>
#include <optional>

namespace core::gpu
{
	struct AccelerationStructure::Impl
	{
		AccelerationStructure& parent;
		const core::gpu::Device* device;

		std::optional<vk::raii::AccelerationStructureKHR> accelerationStructure;
		std::unique_ptr<Buffer> buffer;
		std::unique_ptr<Buffer> scratchBuffer;
		std::unique_ptr<Buffer> instanceBuffer;

		EAccelerationStructureType type;
		std::vector<SAccelerationStructureGeometry> geometries;
		std::vector<SAccelerationStructureInstance> instances;

		vk::BuildAccelerationStructureFlagsKHR buildFlags;
		vk::AccelerationStructureBuildSizesInfoKHR buildSizes;

		explicit Impl(AccelerationStructure& p, const core::gpu::Device* device,
			const SAccelerationStructureCreateInfo& info);

		~Impl();

		uint64_t GetDeviceAddress() const;

		void Build(vk::raii::CommandBuffer& commandBuffer);

		void CreateBottomLevel(const SAccelerationStructureCreateInfo& info);
		void CreateTopLevel(const SAccelerationStructureCreateInfo& info);
		void CreateAccelerationStructureBuffer(vk::DeviceSize size);
		void CreateScratchBuffer(vk::DeviceSize size);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_ACCELERATION_STRUCTURE_H
