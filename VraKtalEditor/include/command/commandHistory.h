#ifndef EDITOR_COMMAND_COMMANDHISTORY_H
#define EDITOR_COMMAND_COMMANDHISTORY_H
#pragma once

#include <vector>
#include <memory>

#include "command.h"

namespace command
{
	class CommandHistory
	{
	private:
		std::vector<std::unique_ptr<Command>> m_history;
		int m_currentIndex = -1;
		size_t m_maxHistorySize = 100;

	public:
		CommandHistory() = default;
		explicit CommandHistory(size_t maxSize) : m_maxHistorySize(maxSize) {}

		void ExecuteCommand(std::unique_ptr<Command> command);

		bool Undo();
		bool Redo();

		bool CanUndo() const { return m_currentIndex >= 0; }
		bool CanRedo() const { return m_currentIndex < static_cast<int>(m_history.size()) - 1; }

		void Clear();

		size_t GetHistorySize() const { return m_history.size(); }

		std::string GetCommandDescription(int index) const;
		std::vector<std::string> GetAllCommandDescriptions() const;

		int GetCurrentIndex() const { return m_currentIndex; }

		void SetMaxHistorySize(size_t maxSize) { m_maxHistorySize = maxSize; }
	};
}

#endif // EDITOR_COMMAND_COMMANDHISTORY_H
