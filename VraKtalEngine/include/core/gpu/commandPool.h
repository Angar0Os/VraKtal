#ifndef VRAKTAL_CORE_GPU_COMMANDPOOL_H
#define VRAKTAL_CORE_GPU_COMMANDPOOL_H
#pragma once

#include <memory>
#include <vector>

#include <core/enum.h>

namespace core::gpu
{
	struct CommandPoolCreateInfo
	{
		uint32_t queueFamilyIndex;
		CommandPoolCreateFlags flags = CommandPoolCreateFlags::None;
	};

	class CommandPool
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		CommandPool(void* device, const CommandPoolCreateInfo& info);
		~CommandPool();

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;

		CommandPool(CommandPool&&) noexcept;
		CommandPool& operator=(CommandPool&&) noexcept;

		std::vector<void*> AllocateCommandBuffers(uint32_t count, bool secondary = false);

		void Reset(bool releaseResources = false);

		void* GetHandle() const;

		Impl& GetImpl();
		const Impl& GetImpl() const;
	};
}

#endif //VRAKTAL_CORE_GPU_COMMANDPOOL_H