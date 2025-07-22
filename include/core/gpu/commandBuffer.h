#ifndef VRAKTAL_RHI_COMMAND_BUFFER_H
#define VRAKTAL_RHI_COMMAND_BUFFER_H
#pragma once

#include <memory>
#include <functional>
#include <queue>

namespace core::gpu::rhi
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void PushFunction(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void Flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
			{
				(*it)();
			}

			deletors.clear();
		}
	};

	class CommandBuffer
	{
		struct Internal;
		std::unique_ptr<Internal> m_Internal;
	};
}

#endif //VRAKTAL_RHI_COMMAND_BUFFER_H
