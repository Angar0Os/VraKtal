#ifndef VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
#define VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
#pragma once

#include <memory>
#include <vector>
#include <cstdint>

namespace core::gpu
{
	class Buffer;
	class AccelerationStructure;

	enum class AccelerationStructureType
	{
		BottomLevel, // (BLAS)
		TopLevel     // (TLAS)
	};

	struct AccelerationStructureGeometry
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

	struct AccelerationStructureInstance
	{
		float transform[12];
		uint32_t instanceCustomIndex = 0;
		uint32_t mask = 0xFF;
		uint32_t instanceShaderBindingTableRecordOffset = 0;
		AccelerationStructure* blas = nullptr;
	};

	struct AccelerationStructureCreateInfo
	{
		AccelerationStructureType type;

		std::vector<AccelerationStructureGeometry> geometries;
		std::vector<AccelerationStructureInstance> instances;

		bool allowUpdate = false;
		bool preferFastTrace = true;
	};

	class AccelerationStructure
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		AccelerationStructure(void* device, void* physicalDevice,
			const AccelerationStructureCreateInfo& info);
		~AccelerationStructure();

		void* GetHandle() const;
		uint64_t GetDeviceAddress() const;

		void Build(void* commandBuffer);

		Impl& GetImpl();
	};
}

#endif //VRAKTAL_CORE_GPU_ACCELERATION_STRUCTURE_H
