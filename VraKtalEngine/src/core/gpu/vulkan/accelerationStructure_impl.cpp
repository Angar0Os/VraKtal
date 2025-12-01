#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/buffer_impl.h"

#include <vulkan/vulkan_raii.hpp>

#include <stdexcept>
#include <cstring>
#include <vector>

using namespace core::gpu;

AccelerationStructure::Impl::Impl(
	AccelerationStructure& p,
	vk::raii::Device& dev,
	vk::raii::PhysicalDevice& physDev,
	const AccelerationStructureCreateInfo& info)
	: parent(p),
	device(dev),
	physicalDevice(physDev),
	type(info.type),
	geometries(info.geometries),
	instances(info.instances),
	buildFlags{}
{
	if (info.preferFastTrace)
		buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	if (info.allowUpdate)
		buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;

	if (type == AccelerationStructureType::BottomLevel)
		CreateBottomLevel(info);
	else
		CreateTopLevel(info);
}

AccelerationStructure::Impl::~Impl() = default;

vk::raii::AccelerationStructureKHR& AccelerationStructure::Impl::GetAccelerationStructure()
{
	return *accelerationStructure;
}

uint64_t AccelerationStructure::Impl::GetDeviceAddress() const
{
	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
	addressInfo.accelerationStructure = **accelerationStructure;
	return device.getAccelerationStructureAddressKHR(addressInfo);
}

Buffer* AccelerationStructure::Impl::GetBuffer() const
{
	return buffer.get();
}

void AccelerationStructure::Impl::CreateAccelerationStructureBuffer(vk::DeviceSize size)
{
	BufferCreateInfo bufferInfo
	{
		.size = static_cast<size_t>(size),
		.usage = BufferUsage::AccelerationStructureStorage | BufferUsage::ShaderDeviceAddress,
		.memoryProperties = MemoryProperty::DeviceLocal
	};

	buffer = std::make_unique<Buffer>(&device, &physicalDevice, bufferInfo);
}

void AccelerationStructure::Impl::CreateScratchBuffer(vk::DeviceSize size)
{
	auto chain = physicalDevice.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
	const auto& accelProps = chain.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();

	vk::DeviceSize alignment = accelProps.minAccelerationStructureScratchOffsetAlignment;
	if (alignment == 0) alignment = 256;

	vk::DeviceSize alignedSize = (size + alignment - 1) & ~(alignment - 1);

	BufferCreateInfo bufferInfo{
		.size = static_cast<size_t>(alignedSize),
		.usage = BufferUsage::StorageBuffer | BufferUsage::ShaderDeviceAddress,
		.memoryProperties = MemoryProperty::DeviceLocal
	};

	scratchBuffer = std::make_unique<Buffer>(&device, &physicalDevice, bufferInfo);

	VkDeviceAddress scratchAddr = scratchBuffer->GetDeviceAddress();
	if (scratchAddr == 0) {
		throw std::runtime_error("Scratch buffer device address is 0");
	}
}

void AccelerationStructure::Impl::CreateBottomLevel(const AccelerationStructureCreateInfo& info)
{
	if (geometries.empty())
		throw std::runtime_error("BLAS requires at least one geometry");

	std::vector<vk::AccelerationStructureGeometryKHR> vkGeometries;
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR> buildRanges;
	std::vector<uint32_t> maxPrimitiveCounts;

	vkGeometries.reserve(geometries.size());
	buildRanges.reserve(geometries.size());
	maxPrimitiveCounts.reserve(geometries.size());

	std::vector<vk::AccelerationStructureGeometryTrianglesDataKHR> trianglesDatas;
	trianglesDatas.reserve(geometries.size());

	for (const auto& geom : geometries)
	{
		if (!geom.vertexBuffer)
			throw std::runtime_error("Geometry missing vertex buffer");

		vk::AccelerationStructureGeometryTrianglesDataKHR trianglesData{};
		trianglesData.vertexFormat = vk::Format::eR32G32B32Sfloat;
		trianglesData.vertexData.deviceAddress = geom.vertexBuffer->GetDeviceAddress();
		trianglesData.vertexStride = geom.vertexStride ? geom.vertexStride : sizeof(float) * 3;
		trianglesData.maxVertex = (geom.vertexCount > 0) ? (geom.vertexCount - 1) : 0;

		if (geom.indexBuffer && geom.indexCount > 0)
		{
			trianglesData.indexType = vk::IndexType::eUint32;
			trianglesData.indexData.deviceAddress = geom.indexBuffer->GetDeviceAddress();
		}
		else
		{
			trianglesData.indexType = vk::IndexType::eNoneKHR;
		}

		if (geom.transformBuffer)
		{
			trianglesData.transformData.deviceAddress = geom.transformBuffer->GetDeviceAddress();
		}

		trianglesDatas.push_back(trianglesData);

		vk::AccelerationStructureGeometryKHR geometry{};
		geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
		geometry.geometry.setTriangles(trianglesDatas.back());
		geometry.flags = geom.opaque ? vk::GeometryFlagBitsKHR::eOpaque : vk::GeometryFlagsKHR{};

		vkGeometries.push_back(geometry);

		vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = geom.triangleCount;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		buildRanges.push_back(rangeInfo);

		maxPrimitiveCounts.push_back(geom.triangleCount);
	}

	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
	buildInfo.setFlags(buildFlags);
	buildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
	buildInfo.setGeometryCount(static_cast<uint32_t>(vkGeometries.size()));
	buildInfo.setPGeometries(vkGeometries.data());

	buildSizes = device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		buildInfo,
		maxPrimitiveCounts
	);

	if (buildSizes.accelerationStructureSize == 0)
		throw std::runtime_error("getAccelerationStructureBuildSizesKHR returned zero accelerationStructureSize");

	CreateAccelerationStructureBuffer(buildSizes.accelerationStructureSize);
	CreateScratchBuffer(buildSizes.buildScratchSize);

	vk::AccelerationStructureCreateInfoKHR createInfo{};
	createInfo.buffer = *buffer->GetImpl().GetBuffer();
	createInfo.size = buildSizes.accelerationStructureSize;
	createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;

	accelerationStructure.emplace(device, createInfo);
}

void AccelerationStructure::Impl::CreateTopLevel(const AccelerationStructureCreateInfo& info)
{
	if (instances.empty())
		throw std::runtime_error("TLAS requires at least one instance");

	BufferCreateInfo instanceBufferInfo{
		.size = sizeof(vk::AccelerationStructureInstanceKHR) * instances.size(),
		.usage = BufferUsage::AccelerationStructureBuildInput | BufferUsage::ShaderDeviceAddress,
		.memoryProperties = MemoryProperty::HostVisible | MemoryProperty::HostCoherent
	};

	instanceBuffer = std::make_unique<Buffer>(&device, &physicalDevice, instanceBufferInfo);

	std::vector<vk::AccelerationStructureInstanceKHR> vkInstances;
	vkInstances.reserve(instances.size());

	for (const auto& instance : instances)
	{
		if (!instance.blas)
			throw std::runtime_error("TLAS instance has null BLAS pointer");

		vk::AccelerationStructureInstanceKHR vkInstance{};
		std::memcpy(&vkInstance.transform, &instance.transform, sizeof(vkInstance.transform));
		vkInstance.instanceCustomIndex = instance.instanceCustomIndex;
		vkInstance.mask = instance.mask;
		vkInstance.instanceShaderBindingTableRecordOffset = instance.instanceShaderBindingTableRecordOffset & 0x00FFFFFF;
		vkInstance.flags = static_cast<VkGeometryInstanceFlagsKHR>(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
		vkInstance.accelerationStructureReference = instance.blas->GetDeviceAddress();

		vkInstances.push_back(vkInstance);
	}

	instanceBuffer->CopyFrom(vkInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * vkInstances.size(), 0);

	vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
	instancesData.setArrayOfPointers(VK_FALSE);
	instancesData.data.deviceAddress = instanceBuffer->GetDeviceAddress();

	if (instancesData.data.deviceAddress == 0)
		throw std::runtime_error("Instance buffer device address is 0");

	vk::AccelerationStructureGeometryKHR geometry{};
	geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
	geometry.geometry.setInstances(instancesData);
	geometry.flags = vk::GeometryFlagsKHR{};

	vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
	buildInfo.setFlags(buildFlags);
	buildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
	buildInfo.setGeometryCount(1);
	buildInfo.setPGeometries(&geometry);

	uint32_t instanceCount = static_cast<uint32_t>(instances.size());

	buildSizes = device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		buildInfo,
		{ instanceCount }
	);

	if (buildSizes.accelerationStructureSize == 0)
		throw std::runtime_error("getAccelerationStructureBuildSizesKHR returned zero accelerationStructureSize (TLAS)");

	CreateAccelerationStructureBuffer(buildSizes.accelerationStructureSize);
	CreateScratchBuffer(buildSizes.buildScratchSize);

	vk::AccelerationStructureCreateInfoKHR createInfo{};
	createInfo.buffer = *buffer->GetImpl().GetBuffer();
	createInfo.size = buildSizes.accelerationStructureSize;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;

	accelerationStructure.emplace(device, createInfo);
}

void AccelerationStructure::Impl::Build(vk::raii::CommandBuffer& commandBuffer)
{
	if (!accelerationStructure.has_value())
	{
		throw std::runtime_error("Acceleration structure not initialized before Build()");
	}

	if (type == AccelerationStructureType::BottomLevel)
	{
		if (geometries.empty())
		{
			throw std::runtime_error("Cannot build BLAS with no geometries");
		}

		std::vector<vk::AccelerationStructureGeometryKHR> vkGeometries;
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR> buildRanges;
		std::vector<vk::AccelerationStructureGeometryTrianglesDataKHR> trianglesDatas;

		vkGeometries.reserve(geometries.size());
		buildRanges.reserve(geometries.size());
		trianglesDatas.reserve(geometries.size());

		for (const auto& geom : geometries)
		{
			if (!geom.vertexBuffer)
			{
				throw std::runtime_error("Geometry has null vertex buffer");
			}

			vk::AccelerationStructureGeometryTrianglesDataKHR trianglesData{};
			trianglesData.vertexFormat = vk::Format::eR32G32B32Sfloat;
			trianglesData.vertexData.deviceAddress = geom.vertexBuffer->GetDeviceAddress();
			trianglesData.vertexStride = geom.vertexStride ? geom.vertexStride : sizeof(float) * 3;
			trianglesData.maxVertex = (geom.vertexCount > 0) ? (geom.vertexCount - 1) : 0;

			if (geom.indexBuffer && geom.indexCount > 0)
			{
				trianglesData.indexType = vk::IndexType::eUint32;
				trianglesData.indexData.deviceAddress = geom.indexBuffer->GetDeviceAddress();
			}
			else
			{
				trianglesData.indexType = vk::IndexType::eNoneKHR;
			}

			if (geom.transformBuffer)
				trianglesData.transformData.deviceAddress = geom.transformBuffer->GetDeviceAddress();

			trianglesDatas.push_back(trianglesData);

			vk::AccelerationStructureGeometryKHR geometry{};
			geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
			geometry.geometry.setTriangles(trianglesDatas.back());
			geometry.flags = geom.opaque ? vk::GeometryFlagBitsKHR::eOpaque : vk::GeometryFlagsKHR{};

			vkGeometries.push_back(geometry);

			vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
			rangeInfo.primitiveCount = geom.triangleCount;
			rangeInfo.primitiveOffset = 0;
			rangeInfo.firstVertex = 0;
			rangeInfo.transformOffset = 0;
			buildRanges.push_back(rangeInfo);
		}

		if (!scratchBuffer)
		{
			throw std::runtime_error("Scratch buffer not created");
		}

		vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
		buildInfo.setFlags(buildFlags);
		buildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
		buildInfo.setSrcAccelerationStructure(VK_NULL_HANDLE);
		buildInfo.setDstAccelerationStructure(**accelerationStructure);
		buildInfo.setGeometryCount(static_cast<uint32_t>(vkGeometries.size()));
		buildInfo.setPGeometries(vkGeometries.data());
		buildInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

		if (buildInfo.scratchData.deviceAddress == 0)
		{
			throw std::runtime_error("Scratch buffer device address is 0");
		}

		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> buildRangePtrs;
		buildRangePtrs.reserve(buildRanges.size());
		for (auto& range : buildRanges)
		{
			buildRangePtrs.push_back(&range);
		}

		commandBuffer.buildAccelerationStructuresKHR({ buildInfo }, buildRangePtrs);
	}
	else
	{
		if (instances.empty())
		{
			throw std::runtime_error("Cannot build TLAS with no instances");
		}

		if (!instanceBuffer)
		{
			throw std::runtime_error("Instance buffer not created");
		}

		if (!scratchBuffer)
		{
			throw std::runtime_error("Scratch buffer not created");
		}

		vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
		instancesData.setArrayOfPointers(VK_FALSE);
		instancesData.data.deviceAddress = instanceBuffer->GetDeviceAddress();

		if (instancesData.data.deviceAddress == 0)
			throw std::runtime_error("Instance buffer device address is 0 (TLAS)");

		vk::AccelerationStructureGeometryKHR geometry{};
		geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
		geometry.geometry.setInstances(instancesData);
		geometry.flags = vk::GeometryFlagsKHR{};

		vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
		buildInfo.setFlags(buildFlags);
		buildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
		buildInfo.setSrcAccelerationStructure(VK_NULL_HANDLE);
		buildInfo.setDstAccelerationStructure(**accelerationStructure);
		buildInfo.setGeometryCount(1);
		buildInfo.setPGeometries(&geometry);
		buildInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

		if (buildInfo.scratchData.deviceAddress == 0)
		{
			throw std::runtime_error("Scratch buffer device address is 0 (TLAS)");
		}

		vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = static_cast<uint32_t>(instances.size());
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;

		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> buildRangePtrs = { &rangeInfo };

		commandBuffer.buildAccelerationStructuresKHR({ buildInfo }, buildRangePtrs);
	}
}

core::gpu::AccelerationStructure::AccelerationStructure(
	void* device, void* physicalDevice,
	const AccelerationStructureCreateInfo& info)
{
	auto& vkDevice = *static_cast<vk::raii::Device*>(device);
	auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);

	m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, info);
}

core::gpu::AccelerationStructure::~AccelerationStructure() = default;

void* core::gpu::AccelerationStructure::GetHandle() const
{
	return reinterpret_cast<void*>(
		static_cast<VkAccelerationStructureKHR>(*m_impl->GetAccelerationStructure())
		);
}

uint64_t core::gpu::AccelerationStructure::GetDeviceAddress() const
{
	return m_impl->GetDeviceAddress();
}

void core::gpu::AccelerationStructure::Build(void* commandBuffer)
{
	auto& vkCmdBuffer = *static_cast<vk::raii::CommandBuffer*>(commandBuffer);
	m_impl->Build(vkCmdBuffer);
}

core::gpu::AccelerationStructure::Impl& core::gpu::AccelerationStructure::GetImpl()
{
	return *m_impl;
}
