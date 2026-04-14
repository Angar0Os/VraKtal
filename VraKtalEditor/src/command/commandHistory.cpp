#include "command/commandHistory.h"
#include <memory>

namespace command
{
	void CommandHistory::ExecuteCommand(std::unique_ptr<Command> command)
	{
		if (!command || !command->CanExecute()) {
			return;
		}

		command->Execute();

		if (m_currentIndex < static_cast<int>(m_history.size()) - 1) {
			m_history.erase(m_history.begin() + m_currentIndex + 1, m_history.end());
		}

		m_history.push_back(std::move(command));
		m_currentIndex++;

		if (m_history.size() > m_maxHistorySize) {
			m_history.erase(m_history.begin());
			m_currentIndex--;
		}

		if (m_onHistoryChanged) {
			m_onHistoryChanged();
		}
	}

	bool CommandHistory::Undo()
	{
		if (!CanUndo()) {
			return false;
		}

		m_history[m_currentIndex]->Undo();
		m_currentIndex--;

		if (m_onHistoryChanged) {
			m_onHistoryChanged();
		}

		return true;
	}

	bool CommandHistory::Redo()
	{
		if (!CanRedo()) {
			return false;
		}

		m_currentIndex++;
		m_history[m_currentIndex]->Execute();

		if (m_onHistoryChanged) {
			m_onHistoryChanged();
		}

		return true;
	}

	void CommandHistory::Clear()
	{
		m_history.clear();
		m_currentIndex = -1;

		if (m_onHistoryChanged) {
			m_onHistoryChanged();
		}
	}

	std::string CommandHistory::GetCommandDescription(int index) const
	{
		if (index < 0 || index >= static_cast<int>(m_history.size())) {
			return "";
		}

		return m_history[index]->GetDescription();
	}

	std::vector<std::string> CommandHistory::GetAllCommandDescriptions() const
	{
		std::vector<std::string> descriptions;
		for (const auto& command : m_history) {
			descriptions.push_back(command->GetDescription());
		}

		return descriptions;
	}
}