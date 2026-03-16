#ifndef EDITOR_COMMAND_COMMAND_H
#define EDITOR_COMMAND_COMMAND_H
#pragma once

#include <string>

namespace command
{
	class Command
	{
	public:
		virtual ~Command() = default;

		virtual void Execute() = 0;

		virtual void Undo() = 0;

		virtual std::string GetDescription() const = 0;

		virtual bool CanExecute() const { return true; }
	};
}

#endif // EDITOR_COMMAND_COMMAND_H
