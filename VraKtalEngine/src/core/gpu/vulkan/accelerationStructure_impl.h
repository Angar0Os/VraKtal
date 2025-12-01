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
	private:
		AccelerationStructure& parent;
		vk::raii::Device& device;
		vk::raii::PhysicalDevice& physicalDevice;

		std::optional<vk::raii::AccelerationStructureKHR> accelerationStructure;
		std::unique_ptr<Buffer> buffer;
		std::unique_ptr<Buffer> scratchBuffer;
		std::unique_ptr<Buffer> instanceBuffer;

		AccelerationStructureType type;
		std::vector<AccelerationStructureGeometry> geometries;
		std::vector<AccelerationStructureInstance> instances;

		vk::BuildAccelerationStructureFlagsKHR buildFlags;
		vk::AccelerationStructureBuildSizesInfoKHR buildSizes;

	public:
		explicit Impl(AccelerationStructure& p, vk::raii::Device& dev,
			vk::raii::PhysicalDevice& physDev,
			const AccelerationStructureCreateInfo& info);

		~Impl();

		vk::raii::AccelerationStructureKHR& GetAccelerationStructure();
		uint64_t GetDeviceAddress() const;
		Buffer* GetBuffer() const;

		void Build(vk::raii::CommandBuffer& commandBuffer);

	private:
		void CreateBottomLevel(const AccelerationStructureCreateInfo& info);
		void CreateTopLevel(const AccelerationStructureCreateInfo& info);
		void CreateAccelerationStructureBuffer(vk::DeviceSize size);
		void CreateScratchBuffer(vk::DeviceSize size);
	};
}

#endif //VRAKTAL_CORE_GPU_VULKAN_ACCELERATION_STRUCTURE_H
